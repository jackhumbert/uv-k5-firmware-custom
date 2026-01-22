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

#ifndef CW_H
#define CW_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "misc.h"

typedef enum {
    CW_INPUT_DISABLED,
    CW_INPUT_ENABLED
} CWState_t;

void CW_EnableInput();
void CW_DisableInput();
void CW_TimeSlice10ms();
void CW_Key_DAH(bool bKeyPressed);
void CW_Key_DIT(bool bKeyPressed);

extern bool gSoundPlaying;
extern CWState_t gCW_State;

// as big as one line with UI_PrintStringSmallNormal
#define CHARS_SENT_SIZE 16

extern uint8_t gCW_CharsTx[CHARS_SENT_SIZE];
extern uint8_t gCW_CharsTxCursor;

// 3 to prevent bad mem access
extern uint64_t gCW_DitsTx[3];
extern uint8_t gCW_DitsTxCursor;

extern char gCW_Values[53];
extern char gCW_Prosigns[11][2];

extern uint8_t gCW_CharsRx[CHARS_SENT_SIZE];
extern uint8_t gCW_CharsRxCursor;

extern uint64_t gCW_DitsRx[3];
extern uint8_t gCW_DitsRxCursor;

enum CW_SYMBOLS {
    CW_SYMBOL_ERROR = ARRAY_SIZE(gCW_Values) + ARRAY_SIZE(gCW_Prosigns),
    CW_SYMBOL_SPACE
};


#endif

// TODO
// * cursor shouldn't advance initially (should stay at pos 0)