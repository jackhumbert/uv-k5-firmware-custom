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
extern CWState_t gCWState;

#endif