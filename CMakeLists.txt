cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(WIN32)
    set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_definitions(_USE_MATH_DEFINES)

    set(MSVC_COMPAT_HEADER "${CMAKE_BINARY_DIR}/msvc_compat.h")
    file(WRITE "${CMAKE_BINARY_DIR}/msvc_compat.h"
            "#pragma once\n#define __attribute__(x)\n")
    add_compile_options(/FI${MSVC_COMPAT_HEADER})
endif()

if(WIN32)
project(motion-fx
        VERSION 1.0.0
        DESCRIPTION "Motion-based FX modulator"
        LANGUAGES CXX C
)
else()
project(motion-fx
        VERSION 1.0.0
        DESCRIPTION "Motion-based FX modulator"
        LANGUAGES CXX C OBJC OBJCXX
)
endif()

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(src)
add_subdirectory(tests)

include(CTest)
enable_testing()

