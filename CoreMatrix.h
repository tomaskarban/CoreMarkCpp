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

using MATDAT = int16_t;
using MATRES = int32_t;

struct mat_params
{
    int32_t N;
    MATDAT *A;
    MATDAT *B;
    MATRES *C;
};

int16_t matrix_test(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val);
int16_t matrix_sum(uint32_t N, MATRES *C, MATDAT clipval);
void matrix_mul_const(uint32_t N, MATRES *C, MATDAT *A, MATDAT val);
void matrix_mul_vect(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_mul_matrix_bitextract(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B);
void matrix_add_const(uint32_t N, MATDAT *A, MATDAT val);

uint32_t core_init_matrix(uint32_t blksize, void *memblk, int32_t seed, mat_params *p);
uint16_t core_bench_matrix(mat_params *p, int16_t seed, uint16_t crc);
