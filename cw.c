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
#include "app/app.h"

// uint32_t values[26] = {
// 0b011101, // ".-",   // A
// 0b0101010111, // "-...", // B
// 0b010111010111, // "-.-.", // C
// 0b01010111, // "-..",  // D
// 0b01, // ".",    // E
// 0b0101110101, // "..-.", // F
// 0b0101110111, // "--.",  // G
// 0b01010101, // "....", // H
// 0b0101, // "..",   // I
// 0b01110111011101, // ".---", // J
// 0b0111010111, // "-.-",  // K
// 0b0101011101, // ".-..", // L
// 0b01110111, // "--",   // M
// 0b010111, // "-.",   // N
// 0b011101110111, // "---",  // O
// 0b010111011101, // ".--.", // P
// 0b01110101110111, // "--.-", // Q
// 0b01011101, // ".-.",  // R
// 0b010101, // "...",  // S
// 0b0111, // "-",    // T
// 0b01110101, // "..-",  // U
// 0b0111010101, // "...-", // V
// 0b0111011101, // ".--",  // W
// 0b011101010111, // "-..-", // X
// 0b01110111010111, // "-.--", // Y
// 0b010101110111, // "--..", // Z
// };

// uint32_t inv_values[26] = {
// 1, // ".",    // E
// 5, // "..",   // I
// 7, // "-",    // T
// 21, // "...",  // S
// 23, // "-.",   // N
// 29, // ".-",   // A
// 85, // "....", // H
// 87, // "-..",  // D
// 93, // ".-.",  // R
// 117, // "..-",  // U
// 119, // "--",   // M
// 1399, // "--..", // Z
// 1495, // "-.-.", // C
// 1501, // ".--.", // P
// 1879, // "-..-", // X
// 1911, // "---",  // O
// 343, // "-...", // B
// 349, // ".-..", // L
// 373, // "..-.", // F
// 375, // "--.",  // G
// 469, // "...-", // V
// 471, // "-.-",  // K
// 477, // ".--",  // W
// 7543, // "--.-", // Q
// 7639, // "-.--", // Y
// 7645, // ".---", // J
// };

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

#define CW_TONE_TUNING_GAIN     66u
#define CW_TONE_FREQ_MULTIPLIER 1353245u
#define CW_SETUP_DELAY_MS       50

static void CW_EnablePulse() {

    // BK4819_DisableDTMF();

    // RADIO_SetTxParameters();

    // LogUart("Start BEEP\n");
    AUDIO_AudioPathOff();

    // if (gCurrentFunction == FUNCTION_POWER_SAVE && gRxIdleMode)
    //     BK4819_RX_TurnOn();

    // SYSTEM_DelayMs(20);

    // ogToneConfig = BK4819_ReadRegister(BK4819_REG_71);

    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE1 | ((1 & 0x7f) << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
    
    // BK4819_PlayTone(700, true); // false is a little lopassy

    // BK4819_EnterTxMute();

    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_MASK_ENABLE_TONE1 | (66u << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));

    // BK4819_WriteRegister(BK4819_REG_71, scale_freq(700));

    // BK4819_SetAF(BK4819_AF_BEEP);

    // BK4819_EnableTXLink();

    // SYSTEM_DelayMs(2);

    RADIO_PrepareTX();

    BK4819_EnterTxMute();
    // gets rid of harmonics on TX
    BK4819_WriteRegister(BK4819_REG_47, (6u << 12) | (BK4819_AF_BEEP << 8) | (1u << 6) | 1);

    // BK4819_SetAF(BK4819_AF_BEEP);
    // BK4819_SetAF(BK4819_AF_AM);

    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE1 | (CW_TONE_TUNING_GAIN << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
    BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE1 | (0 << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
    // BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE2 | (50 << BK4819_REG_70_SHIFT_TONE2_TUNING_GAIN));
    // BK4819_WriteRegister(BK4819_REG_71, (((uint32_t)gCWSettings.tone_hz * CW_TONE_FREQ_MULTIPLIER) + (1u << 16)) >> 17);
    BK4819_WriteRegister(BK4819_REG_71, scale_freq(700));
    // BK4819_WriteRegister(BK4819_REG_72, scale_freq(700));
    // BK4819_EnableTXLink();
    // SYSTEM_DelayMs(CW_SETUP_DELAY_MS);

    // 3360 Hz width with tone
    // 1600 HZ width without tone

    BK4819_WriteRegister(BK4819_REG_30,
        BK4819_REG_30_ENABLE_VCO_CALIB |
        BK4819_REG_30_ENABLE_UNKNOWN   |
        BK4819_REG_30_DISABLE_RX_LINK  |
        BK4819_REG_30_DISABLE_AF_DAC    | // disable beeps
        // BK4819_REG_30_ENABLE_AF_DAC    | // enable beeps
        BK4819_REG_30_ENABLE_DISC_MODE |
        BK4819_REG_30_ENABLE_PLL_VCO   |
        // BK4819_REG_30_ENABLE_PA_GAIN   |
        BK4819_REG_30_DISABLE_PA_GAIN   | // disable PA
        BK4819_REG_30_DISABLE_MIC_ADC  |
        BK4819_REG_30_ENABLE_TX_DSP    |
        // BK4819_REG_30_DISABLE_TX_DSP    | // disables TX
        BK4819_REG_30_DISABLE_RX_DSP);

    // might only affect RX
    // BK4819_WriteRegister(BK4819_REG_43, (0b000 << 12) | (0b000 << 9) | (0b001 << 6) | (0b01 << 4) | (0b0 << 2));
    
    BK4819_SetupPowerAmplifier(gCurrentVfo->TXP_CalculatedSetting, gCurrentVfo->pTX->Frequency);
    AUDIO_AudioPathOn();

    BK4819_ExitTxMute();

    gSoundPlaying = true;
}

static void CW_StartPulse() {
    // BK4819_ExitTxMute();
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, true);
    
    // BK4819_SetupPowerAmplifier(gCurrentVfo->TXP_CalculatedSetting, gCurrentVfo->pTX->Frequency);

    // AUDIO_SetVoiceID(0, VOICE_ID_2_TONE);
    // AUDIO_PlaySingleVoice(0);
    
    BK4819_WriteRegister(BK4819_REG_30,
        BK4819_REG_30_ENABLE_VCO_CALIB |
        BK4819_REG_30_ENABLE_UNKNOWN   |
        BK4819_REG_30_DISABLE_RX_LINK  |
        // BK4819_REG_30_DISABLE_AF_DAC    |
        BK4819_REG_30_ENABLE_AF_DAC    | // enable beep
        BK4819_REG_30_ENABLE_DISC_MODE |
        BK4819_REG_30_ENABLE_PLL_VCO   |
        BK4819_REG_30_ENABLE_PA_GAIN   | // enable PA
        // BK4819_REG_30_DISABLE_PA_GAIN   |
        BK4819_REG_30_DISABLE_MIC_ADC  |
        BK4819_REG_30_ENABLE_TX_DSP    |
        // BK4819_REG_30_DISABLE_TX_DSP    | // disables TX
        BK4819_REG_30_DISABLE_RX_DSP);
    
}

static void CW_EndPulse() {
    // BK4819_EnterTxMute();
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, false);

    BK4819_WriteRegister(BK4819_REG_30,
        BK4819_REG_30_ENABLE_VCO_CALIB |
        BK4819_REG_30_ENABLE_UNKNOWN   |
        BK4819_REG_30_DISABLE_RX_LINK  |
        BK4819_REG_30_DISABLE_AF_DAC    | // disable beep
        // BK4819_REG_30_ENABLE_AF_DAC    |
        BK4819_REG_30_ENABLE_DISC_MODE |
        BK4819_REG_30_ENABLE_PLL_VCO   |
        // BK4819_REG_30_ENABLE_PA_GAIN   |
        BK4819_REG_30_DISABLE_PA_GAIN   | // disable PA
        BK4819_REG_30_DISABLE_MIC_ADC  |
        BK4819_REG_30_ENABLE_TX_DSP    |
        // BK4819_REG_30_DISABLE_TX_DSP    | // disables TX
        BK4819_REG_30_DISABLE_RX_DSP);

    // BK4819_SetupPowerAmplifier(0, 0);

}

static void CW_DisablePulse() {
    gSoundPlaying = false;
    
    // BK4819_EnterTxMute();
    AUDIO_AudioPathOff();

    BK4819_WriteRegister(BK4819_REG_30, 0xC1FE);
    BK4819_SetupPowerAmplifier(0, 0);

    BK4819_WriteRegister(BK4819_REG_70, 0);

    // APP_EndTransmission();
    // FUNCTION_Select(FUNCTION_FOREGROUND);
    RADIO_SetupRegisters(true);
    gFlagEndTransmission = false;

    // SYSTEM_DelayMs(5);
    // BK4819_TurnsOffTones_TurnsOnRX();
    // SYSTEM_DelayMs(5);
    // BK4819_WriteRegister(BK4819_REG_71, ogToneConfig);

    if (gEnableSpeaker)
        AUDIO_AudioPathOn();

    // if (gCurrentFunction == FUNCTION_POWER_SAVE && gRxIdleMode)
    //     BK4819_Sleep();

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
                        if (empty_queue > 5)
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
