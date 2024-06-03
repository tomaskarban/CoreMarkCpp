cmake_minimum_required(VERSION 3.22)

project("CoreMarkCpp")
set(THIS CoreMarkCpp)
set(CMAKE_CXX_STANDARD 20)

set(SOURCES
  "CoreListJoin.cpp"
  "CoreMain.cpp"
  "CoreMatrix.cpp"
  "CoreState.cpp"
  "CoreTime.cpp"
  "CoreUtil.cpp"
)

set(HEADERS
  "CoreListJoin.h"
  "CoreMatrix.h"
  "CoreState.h"
  "CoreTime.h"
  "CoreUtil.h"
)

add_executable(${THIS} ${SOURCES} ${HEADERS})

if(MSVC)
  target_compile_options(${THIS} PRIVATE /MP /permissive- /W4 $<$<CONFIG:Release>:/GF /GL /Gy>)
  target_link_options(${THIS} PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:ICF /OPT:REF>)
else()
  target_compile_options(${THIS} PRIVATE -Wall -Wextra -Wpedantic $<$<CONFIG:Release>:-fdata-sections -ffunction-sections>)
  target_link_options(${THIS} PRIVATE $<$<CONFIG:Release>:-static-libgcc -static-libstdc++ -Wl,--gc-sections>)
endif()
