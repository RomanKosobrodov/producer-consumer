#pragma once

#include "aligned_array.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <latch>

template <typename solution, typename data_type, std::size_t alignment_bytes>
void writer(solution &store,
            std::size_t block_size,
            std::size_t cycles,
            std::latch &thread_latch,
            double &write_time_ns)
{
    spdlog::info("writer starts");

    data_type value{0};
    aligned_array<data_type, alignment_bytes> src(block_size);

    fill(src, value++);
    store.write(src.data(), block_size);

    thread_latch.arrive_and_wait();

    write_time_ns = 0;
    for (size_t k = 1; k < cycles; ++k)
    {
        fill(src, value++);
        const auto t0 = std::chrono::high_resolution_clock::now();
        store.write(src.data(), block_size);
        const auto dt = std::chrono::high_resolution_clock::now() - t0;
        write_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count();
    }

    write_time_ns = write_time_ns / (cycles - 1);
    spdlog::info("Writer terminates. Write time, ns: {:.1f}", write_time_ns);
}

template <typename solution, typename data_type, std::size_t alignment_bytes>
void reader(solution &store,
            std::size_t block_size,
            std::size_t cycles,
            std::size_t index,
            std::latch &thread_latch,
            double &read_time_ns)
{
    spdlog::info("Reader {} starts", index);

    aligned_array<data_type, alignment_bytes> dst(block_size);
    thread_latch.arrive_and_wait();
    read_time_ns = 0;
    std::size_t offset{0};
    const std::size_t total_size = store.size();

    for (size_t k = 0; k < cycles; ++k)
    {
        const auto t0 = std::chrono::high_resolution_clock::now();
        store.read(dst.data(), block_size, offset);
        const auto dt = std::chrono::high_resolution_clock::now() - t0;
        offset += block_size;
        offset = offset % total_size;
        read_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count();
    }

    read_time_ns = read_time_ns / cycles;
    spdlog::info("Reader {} terminates. Read time, ns: {:.1f}", index, read_time_ns);
}

template <typename solution, typename data_type, std::size_t alignment_bytes>
std::vector<double> run_benchmark(std::size_t num_blocks,
                                  std::size_t block_size,
                                  std::size_t num_readers,
                                  std::size_t cycles)
{
    solution store(num_blocks, block_size);
    store.fill(data_type{12345});

    std::latch thread_latch(num_readers + 1);
    std::vector<double> times(num_readers + 1);

    std::thread writer_thread(writer<solution, data_type, alignment_bytes>,
                              std::ref(store),
                              block_size,
                              cycles,
                              std::ref(thread_latch),
                              std::ref(times[0]));

    std::vector<std::thread> readers;
    for (std::size_t k = 0; k < num_readers; ++k)
    {
        readers.emplace_back(reader<solution, data_type, alignment_bytes>,
                             std::ref(store),
                             block_size,
                             cycles,
                             k,
                             std::ref(thread_latch),
                             std::ref(times[k + 1]));
    }

    writer_thread.join();
    for (auto &r : readers)
    {
        r.join();
    }

    return times;
}