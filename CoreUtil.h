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

int32_t parseval(char *valstring);

int32_t get_seed_args(int i, int argc, char *argv[]);

uint16_t crcu8(uint8_t data, uint16_t crc);
uint16_t crc16(int16_t newval, uint16_t crc);
uint16_t crcu16(uint16_t newval, uint16_t crc);
uint16_t crcu32(uint32_t newval, uint16_t crc);
