# CoreMarkCpp

Simplified code from https://github.com/eembc/coremark.

The benchmark algorithm, setup and parameters are unchanged, so the results should be 100% comparable
with the original CoreMark benchmark, provided that CoreMark is compiled with `MULTITHREAD` symbol
value equal to the number of CPU cores on the system. However, the
[original README.md](https://github.com/eembc/coremark/blob/d5fad6bd094899101a4e5fd53af7298160ced6ab/README.md)
clearly states that changing anything outside of `core_portme*` is NOT ALLOWED, and I broke that, obviously.

I made the following modifications:

- Replaced make build with [CMake](https://cmake.org/).
- Reformatted using [clang-format](https://clang.llvm.org/docs/ClangFormat.html).
- Removed platform-specific C code, replaced with portable C++, specifically:
    - replaced fork/socket/pthread with C++
      [std::thread](https://en.cppreference.com/w/cpp/thread/thread),
    - replaced time measurement with
      [std::chrono::steady_clock::now](https://en.cppreference.com/w/cpp/chrono/steady_clock/now),
    - added detection of the number of CPU cores using
      [std::thread::hardware_concurrency](https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency).
- Split the original `coremark.h` into individual header files matching the cpp content, also used
  [IWYU](https://include-what-you-use.org/).
