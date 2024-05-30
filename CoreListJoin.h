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

#include "CoreMatrix.h"
#include <cstdint>
#include <thread>

struct list_data
{
    int16_t data16;
    int16_t idx;
};

struct list_head
{
    list_head *next;
    list_data *info;
};

struct core_results
{
    /* inputs */
    int16_t seed1;       /* Initializing seed */
    int16_t seed2;       /* Initializing seed */
    int16_t seed3;       /* Initializing seed */
    void *memblock[4];   /* Pointer to safe memory location */
    uint32_t size;       /* Size of the data */
    uint32_t iterations; /* Number of iterations to execute */
    uint32_t execs;      /* Bitmask of operations to execute */
    list_head *list;
    mat_params mat;
    /* outputs */
    uint16_t crc;
    uint16_t crclist;
    uint16_t crcmatrix;
    uint16_t crcstate;
    int16_t err;
    /* execution thread */
    std::thread thrd;
};

list_head *core_list_init(uint32_t blksize, list_head *memblock, int16_t seed);
uint16_t core_bench_list(core_results *res, int16_t finder_idx);
void iterate(core_results *res);
void core_start_parallel(core_results *res);
void core_stop_parallel(core_results *res);
