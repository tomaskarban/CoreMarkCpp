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

#include "CoreListJoin.h" // for core_results, core_list_init, core_start_p...
#include "CoreMatrix.h"   // for core_init_matrix
#include "CoreState.h"    // for core_init_state
#include "CoreTime.h"     // for time_in_secs, get_time, start_time, stop_time
#include "CoreUtil.h"     // for get_seed_args, crc16

#include <cstdint> // for uint16_t, uint32_t, int16_t, int32_t, uint8_t
#include <cstdio>  // for printf
#include <cstdlib> // for free, malloc
#include <thread>  // for thread
#include <vector>  // for vector

#define ID_LIST (1 << 0)
#define ID_MATRIX (1 << 1)
#define ID_STATE (1 << 2)
#define ALL_ALGORITHMS_MASK (ID_LIST | ID_MATRIX | ID_STATE)
#define NUM_ALGORITHMS 3

static uint16_t list_known_crc[] = {(uint16_t)0xd4b0, (uint16_t)0x3340, (uint16_t)0x6a79, (uint16_t)0xe714, (uint16_t)0xe3c1};
static uint16_t matrix_known_crc[] = {(uint16_t)0xbe52, (uint16_t)0x1199, (uint16_t)0x5608, (uint16_t)0x1fd7, (uint16_t)0x0747};
static uint16_t state_known_crc[] = {(uint16_t)0x5e47, (uint16_t)0x39bf, (uint16_t)0xe5a4, (uint16_t)0x8e3a, (uint16_t)0x8d84};

#define get_seed_16(x) (int16_t) get_seed_args(x, argc, argv)
#define get_seed_32(x) get_seed_args(x, argc, argv)

int main(int argc, char *argv[])
{
    uint32_t i, j = 0, num_algorithms = 0;
    int32_t known_id = -1, total_errors = 0;
    uint16_t seedcrc = 0;
    CORE_TICKS total_time;

    uint32_t core_count = std::thread::hardware_concurrency();
    if (core_count == 0)
        core_count = 1;

    auto results = std::vector<core_results>(core_count);

    results[0].seed1 = get_seed_16(1);
    results[0].seed2 = get_seed_16(2);
    results[0].seed3 = get_seed_16(3);
    results[0].iterations = get_seed_32(4);
#if CORE_DEBUG
    results[0].iterations = 1;
#endif
    results[0].execs = get_seed_32(5);
    if (results[0].execs == 0)
    {
        results[0].execs = ALL_ALGORITHMS_MASK;
    }

    if ((results[0].seed1 == 0) && (results[0].seed2 == 0) && (results[0].seed3 == 0))
    {
        results[0].seed1 = 0;
        results[0].seed2 = 0;
        results[0].seed3 = 0x66;
    }
    if ((results[0].seed1 == 1) && (results[0].seed2 == 0) && (results[0].seed3 == 0))
    {
        results[0].seed1 = 0x3415;
        results[0].seed2 = 0x3415;
        results[0].seed3 = 0x66;
    }

    for (i = 0; i < core_count; i++)
    {
        int32_t malloc_override = get_seed_16(7);
        results[i].size = (malloc_override > 0) ? malloc_override : 2000;
        results[i].memblock[0] = malloc(results[i].size);
        results[i].seed1 = results[0].seed1;
        results[i].seed2 = results[0].seed2;
        results[i].seed3 = results[0].seed3;
        results[i].err = 0;
        results[i].execs = results[0].execs;
    }

    for (i = 0; i < NUM_ALGORITHMS; i++)
    {
        if ((1 << i) & results[0].execs)
            num_algorithms++;
    }
    for (i = 0; i < core_count; i++)
        results[i].size = results[i].size / num_algorithms;

    for (i = 0; i < NUM_ALGORITHMS; i++)
    {
        uint32_t ctx;
        if ((1 << i) & results[0].execs)
        {
            for (ctx = 0; ctx < core_count; ctx++)
                results[ctx].memblock[i + 1] = (char *)(results[ctx].memblock[0]) + results[0].size * j;
            j++;
        }
    }

    for (i = 0; i < core_count; i++)
    {
        if (results[i].execs & ID_LIST)
        {
            results[i].list = core_list_init(results[0].size, (list_head *)results[i].memblock[1], results[i].seed1);
        }
        if (results[i].execs & ID_MATRIX)
        {
            core_init_matrix(results[0].size, results[i].memblock[2], (int32_t)results[i].seed1 | (((int32_t)results[i].seed2) << 16), &(results[i].mat));
        }
        if (results[i].execs & ID_STATE)
        {
            core_init_state(results[0].size, results[i].seed1, (uint8_t *)results[i].memblock[3]);
        }
    }

    if (results[0].iterations == 0)
    {
        double secs_passed = 0;
        uint32_t divisor;
        results[0].iterations = 1;
        while (secs_passed < 1)
        {
            results[0].iterations *= 10;
            start_time();
            iterate(&results[0]);
            stop_time();
            secs_passed = time_in_secs(get_time());
        }

        divisor = (uint32_t)secs_passed;
        if (divisor == 0)
            divisor = 1;
        results[0].iterations *= 1 + 10 / divisor;
    }

    start_time();

    for (i = 0; i < core_count; i++)
    {
        results[i].iterations = results[0].iterations;
        results[i].execs = results[0].execs;
        core_start_parallel(&results[i]);
    }
    for (i = 0; i < core_count; i++)
    {
        core_stop_parallel(&results[i]);
    }

    stop_time();
    total_time = get_time();

    seedcrc = crc16(results[0].seed1, seedcrc);
    seedcrc = crc16(results[0].seed2, seedcrc);
    seedcrc = crc16(results[0].seed3, seedcrc);
    seedcrc = crc16((int16_t)results[0].size, seedcrc);

    switch (seedcrc)
    {
        case 0x8a02: /* seed1=0, seed2=0, seed3=0x66, size 2000 per algorithm */
            known_id = 0;
            printf("6k performance run parameters for coremark.\n");
            break;
        case 0x7b05: /* seed1=0x3415, seed2=0x3415, seed3=0x66, size 2000 per algorithm */
            known_id = 1;
            printf("6k validation run parameters for coremark.\n");
            break;
        case 0x4eaf: /* seed1=0x8, seed2=0x8, seed3=0x8, size 400 per algorithm */
            known_id = 2;
            printf("Profile generation run parameters for coremark.\n");
            break;
        case 0xe9f5: /* seed1=0, seed2=0, seed3=0x66, size 666 per algorithm */
            known_id = 3;
            printf("2K performance run parameters for coremark.\n");
            break;
        case 0x18f2: /* seed1=0x3415, seed2=0x3415, seed3=0x66, size 666 per algorithm */
            known_id = 4;
            printf("2K validation run parameters for coremark.\n");
            break;
        default:
            total_errors = -1;
            break;
    }

    if (known_id >= 0)
    {
        for (i = 0; i < core_count; i++)
        {
            results[i].err = 0;
            if ((results[i].execs & ID_LIST) && (results[i].crclist != list_known_crc[known_id]))
            {
                printf("[%u]ERROR! list crc 0x%04x - should be 0x%04x\n", i, results[i].crclist, list_known_crc[known_id]);
                results[i].err++;
            }
            if ((results[i].execs & ID_MATRIX) && (results[i].crcmatrix != matrix_known_crc[known_id]))
            {
                printf("[%u]ERROR! matrix crc 0x%04x - should be 0x%04x\n", i, results[i].crcmatrix, matrix_known_crc[known_id]);
                results[i].err++;
            }
            if ((results[i].execs & ID_STATE) && (results[i].crcstate != state_known_crc[known_id]))
            {
                printf("[%u]ERROR! state crc 0x%04x - should be 0x%04x\n", i, results[i].crcstate, state_known_crc[known_id]);
                results[i].err++;
            }
            total_errors += results[i].err;
        }
    }

    printf("CoreMark Size    : %lu\n", (long unsigned)results[0].size);
    printf("Total time (secs): %f\n", time_in_secs(total_time));
    if (time_in_secs(total_time) > 0.0)
        printf("Iterations/Sec   : %f\n", core_count * results[0].iterations / time_in_secs(total_time));

    if (time_in_secs(total_time) < 10.0)
    {
        printf("ERROR! Must execute for at least 10 secs for a valid result!\n");
        total_errors++;
    }

    printf("Iterations       : %lu\n", (long unsigned)core_count * results[0].iterations);
    printf("Parallel threads : %d\n", core_count);

    printf("seedcrc          : 0x%04x\n", seedcrc);
    if (results[0].execs & ID_LIST)
        for (i = 0; i < core_count; i++)
            printf("[%d]crclist       : 0x%04x\n", i, results[i].crclist);
    if (results[0].execs & ID_MATRIX)
        for (i = 0; i < core_count; i++)
            printf("[%d]crcmatrix     : 0x%04x\n", i, results[i].crcmatrix);
    if (results[0].execs & ID_STATE)
        for (i = 0; i < core_count; i++)
            printf("[%d]crcstate      : 0x%04x\n", i, results[i].crcstate);
    for (i = 0; i < core_count; i++)
        printf("[%d]crcfinal      : 0x%04x\n", i, results[i].crc);

    if (total_errors == 0)
    {
        printf("Correct operation validated. See README.md for run and reporting rules.\n");

        if (known_id == 3)
        {
            printf("CoreMark 1.0 : %f\n", core_count * results[0].iterations / time_in_secs(total_time));
        }
    }

    if (total_errors > 0)
        printf("Errors detected\n");
    if (total_errors < 0)
        printf("Cannot validate operation for these seed values, please compare with results on a known platform.\n");

    for (i = 0; i < core_count; i++)
        free(results[i].memblock[0]);

    return 0;
}
