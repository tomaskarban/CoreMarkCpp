/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/

#pragma once

#include <cstdint>

enum CORE_STATE
{
    CORE_START,
    CORE_INVALID,
    CORE_S1,
    CORE_S2,
    CORE_INT,
    CORE_FLOAT,
    CORE_EXPONENT,
    CORE_SCIENTIFIC,
    NUM_CORE_STATES,
};

CORE_STATE core_state_transition(uint8_t **instr, uint32_t *transition_count);

uint16_t core_bench_state(uint32_t blksize, uint8_t *memblock, int16_t seed1, int16_t seed2, int16_t step, uint16_t crc);

void core_init_state(uint32_t size, int16_t seed, uint8_t *p);
