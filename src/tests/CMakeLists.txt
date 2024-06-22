include(CTest)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 
)

FetchContent_MakeAvailable(Catch2)

set(BENCHMARKS_TEST_NAME test_benchmarks)

set(BENCHMARKS_TEST_HEADERS
)

set(BENCHMARKS_TEST_SOURCES
  test_aligned_array.cpp
  test_bad_solution.cpp
	test_main.cpp
)

add_executable(${BENCHMARKS_TEST_NAME}
    ${BENCHMARKS_TEST_HEADERS}
    ${BENCHMARKS_TEST_SOURCES}
)

target_include_directories(${BENCHMARKS_TEST_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/benchmarks/
)

target_link_libraries(${BENCHMARKS_TEST_NAME}
    PRIVATE spdlog::spdlog
    PRIVATE Catch2::Catch2WithMain
)    

add_test(NAME ${BENCHMARKS_TEST_NAME}
         COMMAND  ${BENCHMARKS_TEST_NAME})