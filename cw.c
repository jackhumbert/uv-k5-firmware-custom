/* Copyright 2026 Jack Humbert
 * https://github.com/jackhumbert
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "cw.h"
#include "ui/helper.h"
#include "audio.h"
#include "driver/st7565.h"
#include "driver/bk4819.h"
#include "driver/bk4819-regs.h"
#include "driver/system.h"
#include "misc.h"
#include "functions.h"
#include "debugging.h"
#include "radio.h"

CWState_t gCWState = CW_INPUT_DISABLED;
bool gSoundPlaying = false;

bool ogEnableSpeaker = false;
void CW_EnableInput() {
    gCWState = CW_INPUT_ENABLED;
    ogEnableSpeaker = gEnableSpeaker;
    gEnableSpeaker = true;
}

void CW_DisableInput() {
    gCWState = CW_INPUT_DISABLED;
    gEnableSpeaker = ogEnableSpeaker;
}

bool bDitPressed = false;
bool bDitJustPressed = false;
bool bDitJustReleased = false;
bool bDahPressed = false;
bool bDahJustPressed = false;
bool bDahJustReleased = false;
uint16_t ogToneConfig = 0;

__inline uint16_t scale_freq(const uint16_t freq)
{
//  return (((uint32_t)freq * 1032444u) + 50000u) / 100000u;   // with rounding
    return (((uint32_t)freq * 1353245u) + (1u << 16)) >> 17;   // with rounding
}

static void CW_EnablePulse() {

    // BK4819_DisableDTMF();

    // RADIO_SetTxParameters();

    // LogUart("Start BEEP\n");
    AUDIO_AudioPathOff();

    if (gCurrentFunction == FUNCTION_POWER_SAVE && gRxIdleMode)
        BK4819_RX_TurnOn();

    // SYSTEM_DelayMs(20);

    // ogToneConfig = BK4819_ReadRegister(BK4819_REG_71);

    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE1 | ((1 & 0x7f) << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
    
    BK4819_PlayTone(700, true); // false is a little lopassy

    // BK4819_EnterTxMute();

    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_MASK_ENABLE_TONE1 | (66u << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));

    // BK4819_WriteRegister(BK4819_REG_71, scale_freq(700));

    // BK4819_SetAF(BK4819_AF_BEEP);

    // BK4819_EnableTXLink();

    // SYSTEM_DelayMs(2);
    
    AUDIO_AudioPathOn();

    // BK4819_ExitTxMute();

    gSoundPlaying = true;
}

static void CW_StartPulse() {
    BK4819_ExitTxMute();
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, true);
}

static void CW_EndPulse() {
    BK4819_EnterTxMute();
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, false);
}

static void CW_DisablePulse() {
    gSoundPlaying = false;
    
    // BK4819_EnterTxMute();
    AUDIO_AudioPathOff();

    // SYSTEM_DelayMs(5);
    BK4819_TurnsOffTones_TurnsOnRX();
    // SYSTEM_DelayMs(5);
    // BK4819_WriteRegister(BK4819_REG_71, ogToneConfig);

    if (gEnableSpeaker)
        AUDIO_AudioPathOn();

    if (gCurrentFunction == FUNCTION_POWER_SAVE && gRxIdleMode)
        BK4819_Sleep();

    #ifdef ENABLE_VOX
        gVoxResumeCountdown = 80;
    #endif
    
}

static const uint16_t CW_wpm = 15;
static const uint16_t CW_dit_duration = 120 / CW_wpm; // duration of a dit in 10ms cycles
// static const uint16_t CW_dah_duration = 3 * CW_dit_duration;
// static const uint16_t CW_element_gap = CW_dit_duration; // gap between elements of a character
// static const uint16_t CW_character_gap = 3 * CW_dit_duration; // gap between characters
// static const uint16_t CW_word_gap = 7 * CW_dit_duration; // gap between words

volatile uint16_t gCWCounter = 0;
uint16_t next_tx = 0; // 0 stop, 1 dit, 2 dah
uint16_t last_tx = 0;
uint16_t pulse_length = 0;
uint16_t empty_queue = 0;

enum {
    CW_QUEUE_EMPTY,
    CW_CONSUME_NEXT,
    CW_STARTING_DIT,
    CW_ENDING_DIT,
    CW_STARTING_DAH,
    CW_ENDING_DAH
} cw_state = CW_CONSUME_NEXT;

void CW_TimeSlice10ms() {
    gCWCounter++;
    if (gCWCounter >= CW_dit_duration)
        gCWCounter = 0;
   
    if (bDitJustPressed) {
        next_tx = 1;
    } else if (bDahJustPressed) {
        next_tx = 2;
    } else if (next_tx == 0) {
        // if (bDahPressed && bDitJustReleased) {
        //     next_tx = 2;
        // } else if (bDitPressed && bDahJustReleased) {
        //     next_tx = 1;
        // } else 
        if (bDahPressed && bDitPressed) {
            next_tx = last_tx;
        }
    } else if (bDitJustReleased) {
        if (bDahPressed) {
            next_tx = 2;
        } else {
            next_tx = 0;
        }
    } else if (bDahJustReleased) {
        if (bDitPressed) {
            next_tx = 1;
        } else {
            next_tx = 0;
        }
    }
    bDitJustPressed = false;
    bDahJustPressed = false;
    bDahJustReleased = false;
    bDitJustReleased = false;
    
    if (gCWCounter == 0) {
        if (pulse_length)
            pulse_length--;
        if (!pulse_length) {
            switch (cw_state) {
                case CW_QUEUE_EMPTY:
                    if (gSoundPlaying) {
                        CW_EndPulse();
                        CW_DisablePulse();
                    }
                    [[fallthrough]];
                case CW_CONSUME_NEXT:
                    if (next_tx == 1) {
                        empty_queue = 0;
                        cw_state = CW_STARTING_DIT;
                        last_tx = next_tx;
                        // next_tx = 0;
                        goto StartingDit;
                    } else if (next_tx == 2) {
                        empty_queue = 0;
                        cw_state = CW_STARTING_DAH;
                        last_tx = next_tx;
                        // next_tx = 0;
                        goto StartingDah;
                    } else {
                        if (empty_queue > 10)
                            cw_state = CW_QUEUE_EMPTY;
                        else
                            empty_queue++;
                        last_tx = next_tx;
                        next_tx = 0;
                    }
                    // if (next_tx)
                    break;
                case CW_STARTING_DIT:
StartingDit:
                    if (!gSoundPlaying)
                        CW_EnablePulse();
                    CW_StartPulse();
                    pulse_length = 1;
                    cw_state = CW_ENDING_DIT;
                    // last_tx = next_tx;
                    // next_tx = 0;
                    break;
                case CW_ENDING_DIT:
                    CW_EndPulse();
                    pulse_length = 1;
                    // last_tx = next_tx;
                    // next_tx = 0;
                    cw_state = CW_CONSUME_NEXT;
                    break;
                case CW_STARTING_DAH:
StartingDah:
                    if (!gSoundPlaying)
                        CW_EnablePulse();
                    CW_StartPulse();
                    pulse_length = 3;
                    cw_state = CW_ENDING_DAH;
                    // last_tx = next_tx;
                    // next_tx = 0;
                    break;
                case CW_ENDING_DAH:
                    CW_EndPulse();
                    pulse_length = 1;
                    // last_tx = next_tx;
                    // next_tx = 0;
                    cw_state = CW_CONSUME_NEXT;
                    break;
                default:
                    break;
            } 
        }
    }
    // if (bDitPressed || bDahPressed) {
    //     if (!gSoundPlaying) {
    //         if (gCurrentFunction == FUNCTION_RECEIVE)
    //             return;

    //         if (gCurrentFunction == FUNCTION_MONITOR)
    //             return;

    //         CW_StartPulse();
    //         gSoundPlaying = true;
    //     }
    // } else {
    //     if (gSoundPlaying) {
    //         CW_EndPulse();
    //         gSoundPlaying = false;
    //     }   
    // }
}

void CW_Key_DAH(bool bKeyPressed) {
    if (!bDahPressed && bKeyPressed)
        bDahJustPressed = true;
    if (bDahPressed && !bKeyPressed)
        bDahJustReleased = true;
    bDahPressed = bKeyPressed;
    // if (bDahPressed)
    //     LogUart("DAH Pressed\n");
    // else 
    //     LogUart("DAH Released\n");
}

void CW_Key_DIT(bool bKeyPressed) {
    if (!bDitPressed && bKeyPressed)
        bDitJustPressed = true;
    if (bDitPressed && !bKeyPressed)
        bDitJustReleased = true;
    bDitPressed = bKeyPressed;
    // if (bDitPressed)
    //     LogUart("DIT Pressed\n");
    // else 
    //     LogUart("DIT Released\n");
}
