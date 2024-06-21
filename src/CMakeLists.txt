cmake_minimum_required(VERSION 3.25.1)

project(producer_consumer VERSION 0.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "do not build shared libraries" FORCE)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if (UNIX)
    add_compile_options("$<$<CONFIG:DEBUG>:-O1>" "$<$<CONFIG:DEBUG>:-fsanitize=undefined,address,leak>")
    add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address,leak>")
endif(UNIX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO  "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG 60f5cb73a88ea4e2ee6ee3e5a6431bd6549d6484
    OVERRIDE_FIND_PACKAGE 
)

FetchContent_MakeAvailable(spdlog)

add_subdirectory(benchmarks)

add_subdirectory(tests)




