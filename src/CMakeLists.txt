cmake_minimum_required(VERSION 3.25.1)

project(producer_consumer VERSION 0.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "do not build shared libraries" FORCE)

if (UNIX)
    add_compile_options("$<$<CONFIG:RelWithDebInfo>:-O2>" "$<$<CONFIG:RelWithDebInfo>:-fsanitize=thread>")
    add_link_options("$<$<CONFIG:RelWithDebInfo>:-fsanitize=thread>")
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

FetchContent_Declare(
    zmq
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG        v4.3.5
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG        v4.10.0
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG 44739d3023155fbf5f93937e22a68c9a429f7b89
    OVERRIDE_FIND_PACKAGE 
)

set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "Do not build CppZmq tests" FORCE)

FetchContent_MakeAvailable(spdlog zmq cppzmq cxxopts)

add_subdirectory(benchmarks)

add_subdirectory(tests)
