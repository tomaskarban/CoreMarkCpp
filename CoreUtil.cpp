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

#include "CoreUtil.h"

int32_t parseval(char *valstring)
{
    int32_t retval = 0;
    int32_t neg = 1;
    bool hexmode = false;
    if (*valstring == '-')
    {
        neg = -1;
        valstring++;
    }
    if ((valstring[0] == '0') && (valstring[1] == 'x'))
    {
        hexmode = true;
        valstring += 2;
    }
    /* first look for digits */
    if (hexmode)
    {
        while (((*valstring >= '0') && (*valstring <= '9')) || ((*valstring >= 'a') && (*valstring <= 'f')))
        {
            int32_t digit = *valstring - '0';
            if (digit > 9)
                digit = 10 + *valstring - 'a';
            retval *= 16;
            retval += digit;
            valstring++;
        }
    }
    else
    {
        while ((*valstring >= '0') && (*valstring <= '9'))
        {
            int32_t digit = *valstring - '0';
            retval *= 10;
            retval += digit;
            valstring++;
        }
    }
    /* now add qualifiers */
    if (*valstring == 'K')
        retval *= 1024;
    if (*valstring == 'M')
        retval *= 1024 * 1024;

    retval *= neg;
    return retval;
}

int32_t get_seed_args(int i, int argc, char *argv[])
{
    if (i < argc)
        return parseval(argv[i]);
    return 0;
}

uint16_t crcu8(uint8_t data, uint16_t crc)
{
    uint8_t i = 0, x16 = 0, carry = 0;

    for (i = 0; i < 8; i++)
    {
        x16 = (uint8_t)((data & 1) ^ ((uint8_t)crc & 1));
        data >>= 1;

        if (x16 == 1)
        {
            crc ^= 0x4002;
            carry = 1;
        }
        else
            carry = 0;
        crc >>= 1;
        if (carry)
            crc |= 0x8000;
        else
            crc &= 0x7fff;
    }
    return crc;
}

uint16_t crcu16(uint16_t newval, uint16_t crc)
{
    crc = crcu8((uint8_t)(newval), crc);
    crc = crcu8((uint8_t)((newval) >> 8), crc);
    return crc;
}

uint16_t crcu32(uint32_t newval, uint16_t crc)
{
    crc = crc16((int16_t)newval, crc);
    crc = crc16((int16_t)(newval >> 16), crc);
    return crc;
}

uint16_t crc16(int16_t newval, uint16_t crc)
{
    return crcu16((uint16_t)newval, crc);
}
