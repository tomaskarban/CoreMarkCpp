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

#include "CoreMatrix.h"

#include "CoreUtil.h"

#define matrix_test_next(x) (x + 1)
#define matrix_clip(x, y) ((y) ? (x) & 0x0ff : (x) & 0x0ffff)
#define matrix_big(x) (0xf000 | (x))
#define bit_extract(x, from, to) (((x) >> (from)) & (~(0xffffffff << (to))))

/* align an offset to point to a 32b value */
#define align_mem(x) (void *)(4 + (((intptr_t)(x)-1) & ~3))

#if CORE_DEBUG
void printmat(MATDAT *A, uint32_t N, char *name)
{
    uint32_t i, j;
    printf("Matrix %s [%dx%d]:\n", name, N, N);
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (j != 0)
                printf(",");
            printf("%d", A[i * N + j]);
        }
        printf("\n");
    }
}

void printmatC(MATRES *C, uint32_t N, char *name)
{
    uint32_t i, j;
    printf("Matrix %s [%dx%d]:\n", name, N, N);
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (j != 0)
                printf(",");
            printf("%d", C[i * N + j]);
        }
        printf("\n");
    }
}
#endif

uint16_t core_bench_matrix(mat_params *p, int16_t seed, uint16_t crc)
{
    uint32_t N = p->N;
    MATRES *C = p->C;
    MATDAT *A = p->A;
    MATDAT *B = p->B;
    MATDAT val = (MATDAT)seed;

    crc = crc16(matrix_test(N, C, A, B, val), crc);

    return crc;
}

int16_t matrix_test(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B, MATDAT val)
{
    uint16_t crc = 0;
    MATDAT clipval = matrix_big(val);

    matrix_add_const(N, A, val); /* make sure data changes  */
#if CORE_DEBUG
    printmat(A, N, "matrix_add_const");
#endif
    matrix_mul_const(N, C, A, val);
    crc = crc16(matrix_sum(N, C, clipval), crc);
#if CORE_DEBUG
    printmatC(C, N, "matrix_mul_const");
#endif
    matrix_mul_vect(N, C, A, B);
    crc = crc16(matrix_sum(N, C, clipval), crc);
#if CORE_DEBUG
    printmatC(C, N, "matrix_mul_vect");
#endif
    matrix_mul_matrix(N, C, A, B);
    crc = crc16(matrix_sum(N, C, clipval), crc);
#if CORE_DEBUG
    printmatC(C, N, "matrix_mul_matrix");
#endif
    matrix_mul_matrix_bitextract(N, C, A, B);
    crc = crc16(matrix_sum(N, C, clipval), crc);
#if CORE_DEBUG
    printmatC(C, N, "matrix_mul_matrix_bitextract");
#endif

    matrix_add_const(N, A, -val); /* return matrix to initial value */
    return crc;
}

uint32_t core_init_matrix(uint32_t blksize, void *memblk, int32_t seed, mat_params *p)
{
    uint32_t N = 0;
    MATDAT *A;
    MATDAT *B;
    int32_t order = 1;
    MATDAT val;
    uint32_t i = 0, j = 0;
    if (seed == 0)
        seed = 1;
    while (j < blksize)
    {
        i++;
        j = i * i * 2 * 4;
    }
    N = i - 1;
    A = (MATDAT *)align_mem(memblk);
    B = A + N * N;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            seed = ((order * seed) % 65536);
            val = (MATDAT)(seed + order);
            val = matrix_clip(val, 0);
            B[i * N + j] = val;
            val = (MATDAT)(val + order);
            val = matrix_clip(val, 1);
            A[i * N + j] = val;
            order++;
        }
    }

    p->A = A;
    p->B = B;
    p->C = (MATRES *)align_mem(B + N * N);
    p->N = N;
#if CORE_DEBUG
    printmat(A, N, "A");
    printmat(B, N, "B");
#endif
    return N;
}

int16_t matrix_sum(uint32_t N, MATRES *C, MATDAT clipval)
{
    MATRES tmp = 0, prev = 0, cur = 0;
    int16_t ret = 0;
    uint32_t i, j;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            cur = C[i * N + j];
            tmp += cur;
            if (tmp > clipval)
            {
                ret += 10;
                tmp = 0;
            }
            else
            {
                ret += (cur > prev) ? 1 : 0;
            }
            prev = cur;
        }
    }
    return ret;
}

void matrix_mul_const(uint32_t N, MATRES *C, MATDAT *A, MATDAT val)
{
    uint32_t i, j;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            C[i * N + j] = (MATRES)A[i * N + j] * (MATRES)val;
        }
    }
}

void matrix_add_const(uint32_t N, MATDAT *A, MATDAT val)
{
    uint32_t i, j;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            A[i * N + j] += val;
        }
    }
}

void matrix_mul_vect(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B)
{
    uint32_t i, j;
    for (i = 0; i < N; i++)
    {
        C[i] = 0;
        for (j = 0; j < N; j++)
        {
            C[i] += (MATRES)A[i * N + j] * (MATRES)B[j];
        }
    }
}

void matrix_mul_matrix(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B)
{
    uint32_t i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            C[i * N + j] = 0;
            for (k = 0; k < N; k++)
            {
                C[i * N + j] += (MATRES)A[i * N + k] * (MATRES)B[k * N + j];
            }
        }
    }
}

void matrix_mul_matrix_bitextract(uint32_t N, MATRES *C, MATDAT *A, MATDAT *B)
{
    uint32_t i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            C[i * N + j] = 0;
            for (k = 0; k < N; k++)
            {
                MATRES tmp = (MATRES)A[i * N + k] * (MATRES)B[k * N + j];
                C[i * N + j] += bit_extract(tmp, 2, 4) * bit_extract(tmp, 5, 7);
            }
        }
    }
}
