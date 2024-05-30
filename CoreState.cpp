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

#include "CoreState.h"

#include "CoreUtil.h"

uint16_t core_bench_state(uint32_t blksize, uint8_t *memblock, int16_t seed1, int16_t seed2, int16_t step, uint16_t crc)
{
    uint32_t final_counts[NUM_CORE_STATES];
    uint32_t track_counts[NUM_CORE_STATES];
    uint8_t *p = memblock;
    uint32_t i;

#if CORE_DEBUG
    printf("State Bench: %d,%d,%d,%04x\n", seed1, seed2, step, crc);
#endif

    for (i = 0; i < NUM_CORE_STATES; i++)
    {
        final_counts[i] = track_counts[i] = 0;
    }
    /* run the state machine over the input */
    while (*p != 0)
    {
        CORE_STATE fstate = core_state_transition(&p, track_counts);
        final_counts[fstate]++;

#if CORE_DEBUG
        printf("%d,", fstate);
    }
    printf("\n");
#else
    }
#endif

    p = memblock;
    while (p < (memblock + blksize))
    { /* insert some corruption */
        if (*p != ',')
            *p ^= (uint8_t)seed1;
        p += step;
    }
    p = memblock;
    /* run the state machine over the input again */
    while (*p != 0)
    {
        CORE_STATE fstate = core_state_transition(&p, track_counts);
        final_counts[fstate]++;

#if CORE_DEBUG
        printf("%d,", fstate);
    }
    printf("\n");
#else
    }
#endif

    p = memblock;
    while (p < (memblock + blksize))
    {
        /* undo corruption if seed1 and seed2 are equal */
        if (*p != ',')
            *p ^= (uint8_t)seed2;
        p += step;
    }
    /* end timing */
    for (i = 0; i < NUM_CORE_STATES; i++)
    {
        crc = crcu32(final_counts[i], crc);
        crc = crcu32(track_counts[i], crc);
    }
    return crc;
}

/* Default initialization patterns */
static uint8_t *intpat[4] = {(uint8_t *)"5012", (uint8_t *)"1234", (uint8_t *)"-874", (uint8_t *)"+122"};
static uint8_t *floatpat[4] = {(uint8_t *)"35.54400", (uint8_t *)".1234500", (uint8_t *)"-110.700", (uint8_t *)"+0.64400"};
static uint8_t *scipat[4] = {(uint8_t *)"5.500e+3", (uint8_t *)"-.123e-2", (uint8_t *)"-87e+832", (uint8_t *)"+0.6e-12"};
static uint8_t *errpat[4] = {(uint8_t *)"T0.3e-1F", (uint8_t *)"-T.T++Tq", (uint8_t *)"1T3.4e4z", (uint8_t *)"34.0e-T^"};

void core_init_state(uint32_t size, int16_t seed, uint8_t *p)
{
    uint32_t total = 0, next = 0, i;
    uint8_t *buf = nullptr;

#if CORE_DEBUG
    uint8_t *start = p;
    printf("State: %d,%d\n", size, seed);
#endif

    size--;
    next = 0;
    while ((total + next + 1) < size)
    {
        if (next > 0)
        {
            for (i = 0; i < next; i++)
                *(p + total + i) = buf[i];
            *(p + total + i) = ',';
            total += next + 1;
        }
        seed++;
        switch (seed & 0x7)
        {
            case 0: /* int */
            case 1: /* int */
            case 2: /* int */
                buf = intpat[(seed >> 3) & 0x3];
                next = 4;
                break;
            case 3: /* float */
            case 4: /* float */
                buf = floatpat[(seed >> 3) & 0x3];
                next = 8;
                break;
            case 5: /* scientific */
            case 6: /* scientific */
                buf = scipat[(seed >> 3) & 0x3];
                next = 8;
                break;
            case 7: /* invalid */
                buf = errpat[(seed >> 3) & 0x3];
                next = 8;
                break;
            default: /* Never happen, just to make some compilers happy */
                break;
        }
    }
    size++;
    while (total < size)
    {
        /* fill the rest with 0 */
        *(p + total) = 0;
        total++;
    }

#if CORE_DEBUG
    printf("State Input: %s\n", start);
#endif
}

static bool ee_isdigit(uint8_t c)
{
    return (c >= '0') && (c <= '9');
}

CORE_STATE core_state_transition(uint8_t **instr, uint32_t *transition_count)
{
    uint8_t *str = *instr;
    uint8_t NEXT_SYMBOL;
    CORE_STATE state = CORE_START;
    for (; *str && state != CORE_INVALID; str++)
    {
        NEXT_SYMBOL = *str;
        if (NEXT_SYMBOL == ',')
        {
            /* end of this input */
            str++;
            break;
        }
        switch (state)
        {
            case CORE_START:
                if (ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_INT;
                }
                else if (NEXT_SYMBOL == '+' || NEXT_SYMBOL == '-')
                {
                    state = CORE_S1;
                }
                else if (NEXT_SYMBOL == '.')
                {
                    state = CORE_FLOAT;
                }
                else
                {
                    state = CORE_INVALID;
                    transition_count[CORE_INVALID]++;
                }
                transition_count[CORE_START]++;
                break;
            case CORE_S1:
                if (ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_INT;
                    transition_count[CORE_S1]++;
                }
                else if (NEXT_SYMBOL == '.')
                {
                    state = CORE_FLOAT;
                    transition_count[CORE_S1]++;
                }
                else
                {
                    state = CORE_INVALID;
                    transition_count[CORE_S1]++;
                }
                break;
            case CORE_INT:
                if (NEXT_SYMBOL == '.')
                {
                    state = CORE_FLOAT;
                    transition_count[CORE_INT]++;
                }
                else if (!ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_INVALID;
                    transition_count[CORE_INT]++;
                }
                break;
            case CORE_FLOAT:
                if (NEXT_SYMBOL == 'E' || NEXT_SYMBOL == 'e')
                {
                    state = CORE_S2;
                    transition_count[CORE_FLOAT]++;
                }
                else if (!ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_INVALID;
                    transition_count[CORE_FLOAT]++;
                }
                break;
            case CORE_S2:
                if (NEXT_SYMBOL == '+' || NEXT_SYMBOL == '-')
                {
                    state = CORE_EXPONENT;
                    transition_count[CORE_S2]++;
                }
                else
                {
                    state = CORE_INVALID;
                    transition_count[CORE_S2]++;
                }
                break;
            case CORE_EXPONENT:
                if (ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_SCIENTIFIC;
                    transition_count[CORE_EXPONENT]++;
                }
                else
                {
                    state = CORE_INVALID;
                    transition_count[CORE_EXPONENT]++;
                }
                break;
            case CORE_SCIENTIFIC:
                if (!ee_isdigit(NEXT_SYMBOL))
                {
                    state = CORE_INVALID;
                    transition_count[CORE_INVALID]++;
                }
                break;
            default:
                break;
        }
    }
    *instr = str;
    return state;
}
