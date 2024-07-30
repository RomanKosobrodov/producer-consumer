set(BENCHMARKS "benchmarks")

set(BENCHMARKS_HEADER_FILES
    aligned_array.hpp
    benchmark.hpp
    seqlock_solution.hpp
    storage.hpp
    synchronised_solution.hpp
    unsynchronised_solution.hpp
    zmq_benchmark.hpp
)

set(BENCHMARKS_SOURCE_FILES
    main.cpp
)

add_executable(${BENCHMARKS} 
    ${BENCHMARKS_HEADER_FILES}
    ${BENCHMARKS_SOURCE_FILES}
)

target_link_libraries(${BENCHMARKS}
    PRIVATE spdlog::spdlog
    PRIVATE cppzmq
)

target_compile_definitions(${BENCHMARKS} PRIVATE CMAKE_EXPORT_COMPILE_COMMANDS=1)
