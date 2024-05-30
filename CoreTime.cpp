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

#include "CoreTime.h"

#include <ratio> // for ratio

static std::chrono::steady_clock::time_point start_time_val, stop_time_val;

void start_time(void)
{
    start_time_val = std::chrono::steady_clock::now();
}

void stop_time(void)
{
    stop_time_val = std::chrono::steady_clock::now();
}

CORE_TICKS get_time(void)
{
    return stop_time_val - start_time_val;
}

double time_in_secs(CORE_TICKS ticks)
{
    using double_seconds = std::chrono::duration<double, std::ratio<1>>;
    return std::chrono::duration_cast<double_seconds>(ticks).count();
}
