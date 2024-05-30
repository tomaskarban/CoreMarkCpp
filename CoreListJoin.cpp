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

#include "CoreListJoin.h"

#include "CoreMatrix.h" // for core_bench_matrix
#include "CoreState.h"  // for core_bench_state
#include "CoreUtil.h"   // for crcu16, crc16

#include <utility> // for move

list_head *core_list_find(list_head *list, list_data *info);
list_head *core_list_reverse(list_head *list);
list_head *core_list_remove(list_head *item);
list_head *core_list_undo_remove(list_head *item_removed, list_head *item_modified);
list_head *core_list_insert_new(list_head *insert_point, list_data *info, list_head **memblock, list_data **datablock, list_head *memblock_end,
                                list_data *datablock_end);
using list_cmp = int32_t (*)(list_data *a, list_data *b, core_results *res);
list_head *core_list_mergesort(list_head *list, list_cmp cmp, core_results *res);

int16_t calc_func(int16_t *pdata, core_results *res)
{
    int16_t data = *pdata;
    int16_t retval;
    uint8_t optype = (data >> 7) & 1;
    if (optype)
        return (data & 0x007f);
    else
    {
        int16_t flag = data & 0x7;
        int16_t dtype = ((data >> 3) & 0xf);
        dtype |= dtype << 4;
        switch (flag)
        {
            case 0:
                if (dtype < 0x22)
                    dtype = 0x22;
                retval = core_bench_state(res->size, (uint8_t *)res->memblock[3], res->seed1, res->seed2, dtype, res->crc);
                if (res->crcstate == 0)
                    res->crcstate = retval;
                break;
            case 1:
                retval = core_bench_matrix(&(res->mat), dtype, res->crc);
                if (res->crcmatrix == 0)
                    res->crcmatrix = retval;
                break;
            default:
                retval = data;
                break;
        }
        res->crc = crcu16(retval, res->crc);
        retval &= 0x007f;
        *pdata = (data & 0xff00) | 0x0080 | retval;
        return retval;
    }
}

int32_t cmp_complex(list_data *a, list_data *b, core_results *res)
{
    int16_t val1 = calc_func(&(a->data16), res);
    int16_t val2 = calc_func(&(b->data16), res);
    return val1 - val2;
}

int32_t cmp_idx(list_data *a, list_data *b, core_results *res)
{
    if (res == nullptr)
    {
        a->data16 = (a->data16 & 0xff00) | (0x00ff & (a->data16 >> 8));
        b->data16 = (b->data16 & 0xff00) | (0x00ff & (b->data16 >> 8));
    }
    return a->idx - b->idx;
}

void copy_info(list_data *to, list_data *from)
{
    to->data16 = from->data16;
    to->idx = from->idx;
}

uint16_t core_bench_list(core_results *res, int16_t finder_idx)
{
    uint16_t retval = 0;
    uint16_t found = 0, missed = 0;
    list_head *list = res->list;
    int16_t find_num = res->seed3;
    list_head *this_find;
    list_head *finder, *remover;
    list_data info = {0, 0};
    int16_t i;

    info.idx = finder_idx;
    for (i = 0; i < find_num; i++)
    {
        info.data16 = (i & 0xff);
        this_find = core_list_find(list, &info);
        list = core_list_reverse(list);
        if (this_find == nullptr)
        {
            missed++;
            retval += (list->next->info->data16 >> 8) & 1;
        }
        else
        {
            found++;
            if (this_find->info->data16 & 0x1)
                retval += (this_find->info->data16 >> 9) & 1;
            if (this_find->next != nullptr)
            {
                finder = this_find->next;
                this_find->next = finder->next;
                finder->next = list->next;
                list->next = finder;
            }
        }
        if (info.idx >= 0)
            info.idx++;

#if CORE_DEBUG
        printf("List find %d: [%d,%d,%d]\n", i, retval, missed, found);
#endif
    }
    retval += found * 4 - missed;
    if (finder_idx > 0)
        list = core_list_mergesort(list, cmp_complex, res);
    remover = core_list_remove(list->next);
    finder = core_list_find(list, &info);
    if (!finder)
        finder = list->next;
    while (finder)
    {
        retval = crc16(list->info->data16, retval);
        finder = finder->next;
    }
#if CORE_DEBUG
    printf("List sort 1: %04x\n", retval);
#endif
    remover = core_list_undo_remove(remover, list->next);
    list = core_list_mergesort(list, cmp_idx, nullptr);
    finder = list->next;
    while (finder)
    {
        retval = crc16(list->info->data16, retval);
        finder = finder->next;
    }
#if CORE_DEBUG
    printf("List sort 2: %04x\n", retval);
#endif
    return retval;
}

list_head *core_list_init(uint32_t blksize, list_head *memblock, int16_t seed)
{
    uint32_t per_item = 16 + sizeof(list_data);
    uint32_t size = (blksize / per_item) - 2;
    list_head *memblock_end = memblock + size;
    list_data *datablock = (list_data *)(memblock_end);
    list_data *datablock_end = datablock + size;
    uint32_t i;
    list_head *finder, *list = memblock;
    list_data info{0, 0};

    list->next = nullptr;
    list->info = datablock;
    list->info->idx = 0x0000;
    list->info->data16 = (int16_t)-32640;
    memblock++;
    datablock++;
    info.idx = 0x7fff;
    info.data16 = (int16_t)-1;
    core_list_insert_new(list, &info, &memblock, &datablock, memblock_end, datablock_end);

    for (i = 0; i < size; i++)
    {
        uint16_t datpat = ((uint16_t)(seed ^ i) & 0xf);
        uint16_t dat = (datpat << 3) | (i & 0x7);
        info.data16 = (dat << 8) | dat;
        core_list_insert_new(list, &info, &memblock, &datablock, memblock_end, datablock_end);
    }
    finder = list->next;
    i = 1;
    while (finder->next != nullptr)
    {
        if (i < size / 5)
            finder->info->idx = (int16_t)i++;
        else
        {
            uint16_t pat = (uint16_t)(i++ ^ seed);
            finder->info->idx = 0x3fff & (((i & 0x07) << 8) | pat);
        }
        finder = finder->next;
    }
    list = core_list_mergesort(list, cmp_idx, nullptr);
#if CORE_DEBUG
    printf("Initialized list:\n");
    finder = list;
    while (finder)
    {
        printf("[%04x,%04x]", finder->info->idx, (uint16_t)finder->info->data16);
        finder = finder->next;
    }
    printf("\n");
#endif
    return list;
}

list_head *core_list_insert_new(list_head *insert_point, list_data *info, list_head **memblock, list_data **datablock, list_head *memblock_end,
                                list_data *datablock_end)
{
    list_head *newitem;

    if ((*memblock + 1) >= memblock_end)
        return nullptr;
    if ((*datablock + 1) >= datablock_end)
        return nullptr;

    newitem = *memblock;
    (*memblock)++;
    newitem->next = insert_point->next;
    insert_point->next = newitem;

    newitem->info = *datablock;
    (*datablock)++;
    copy_info(newitem->info, info);

    return newitem;
}

list_head *core_list_remove(list_head *item)
{
    list_data *tmp;
    list_head *ret = item->next;
    /* swap data pointers */
    tmp = item->info;
    item->info = ret->info;
    ret->info = tmp;
    /* and eliminate item */
    item->next = item->next->next;
    ret->next = nullptr;
    return ret;
}

list_head *core_list_undo_remove(list_head *item_removed, list_head *item_modified)
{
    list_data *tmp;
    /* swap data pointers */
    tmp = item_removed->info;
    item_removed->info = item_modified->info;
    item_modified->info = tmp;
    /* and insert item */
    item_removed->next = item_modified->next;
    item_modified->next = item_removed;
    return item_removed;
}

list_head *core_list_find(list_head *list, list_data *info)
{
    if (info->idx >= 0)
    {
        while (list && (list->info->idx != info->idx))
            list = list->next;
        return list;
    }
    else
    {
        while (list && ((list->info->data16 & 0xff) != info->data16))
            list = list->next;
        return list;
    }
}

list_head *core_list_reverse(list_head *list)
{
    list_head *next = nullptr, *tmp;
    while (list)
    {
        tmp = list->next;
        list->next = next;
        next = list;
        list = tmp;
    }
    return next;
}

list_head *core_list_mergesort(list_head *list, list_cmp cmp, core_results *res)
{
    list_head *p, *q, *e, *tail;
    int32_t insize, nmerges, psize, qsize, i;

    insize = 1;

    while (1)
    {
        p = list;
        list = nullptr;
        tail = nullptr;

        nmerges = 0;

        while (p)
        {
            nmerges++;
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++)
            {
                psize++;
                q = q->next;
                if (!q)
                    break;
            }

            qsize = insize;

            while (psize > 0 || (qsize > 0 && q))
            {
                if (psize == 0)
                {
                    e = q;
                    q = q->next;
                    qsize--;
                }
                else if (qsize == 0 || !q)
                {
                    e = p;
                    p = p->next;
                    psize--;
                }
                else if (cmp(p->info, q->info, res) <= 0)
                {
                    e = p;
                    p = p->next;
                    psize--;
                }
                else
                {
                    e = q;
                    q = q->next;
                    qsize--;
                }

                if (tail)
                {
                    tail->next = e;
                }
                else
                {
                    list = e;
                }
                tail = e;
            }

            p = q;
        }

        if (tail != nullptr)
            tail->next = nullptr;

        if (nmerges <= 1)
            return list;

        insize *= 2;
    }
}

void iterate(core_results *res)
{
    uint32_t i;
    uint16_t crc;
    uint32_t iterations = res->iterations;
    res->crc = 0;
    res->crclist = 0;
    res->crcmatrix = 0;
    res->crcstate = 0;

    for (i = 0; i < iterations; i++)
    {
        crc = core_bench_list(res, 1);
        res->crc = crcu16(crc, res->crc);
        crc = core_bench_list(res, -1);
        res->crc = crcu16(crc, res->crc);
        if (i == 0)
            res->crclist = res->crc;
    }
}

void core_start_parallel(core_results *res)
{
    std::thread t(iterate, res);
    res->thrd = std::move(t);
}

void core_stop_parallel(core_results *res)
{
    res->thrd.join();
}
