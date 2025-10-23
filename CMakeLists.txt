cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:char8_t->")
endif()

project(motion-fx
        VERSION 1.0.0
        DESCRIPTION "Motion-based FX modulator"
        LANGUAGES CXX C OBJC OBJCXX
)

add_subdirectory(src)

include(CTest)
enable_testing()

