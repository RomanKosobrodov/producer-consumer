#pragma once

#include "aligned_array.hpp"

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <vector>
#include <thread>
#include <barrier>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

template <typename data_type, std::size_t alignment_bytes>
void zmq_writer(zmq::context_t &ctx,
                std::size_t block_size,
                std::size_t cycles,
                std::barrier<> &thread_barrier,
                double &write_time_ns)
{
    spdlog::info("ZMQ writer starts");

    zmq::socket_t publisher(ctx, zmq::socket_type::pub);
    publisher.bind("inproc://endpoint");

    data_type value{0};
    aligned_array<data_type, alignment_bytes> src(block_size);
    zmq::mutable_buffer buffer(src.data(), src.size() * sizeof(data_type));
    zmq::send_result_t result;

    thread_barrier.arrive_and_wait();

    write_time_ns = 0;
    for (size_t k = 1; k < cycles; ++k)
    {
        fill(src, value++);
        const auto t0 = std::chrono::high_resolution_clock::now();
        result = publisher.send(buffer);
        const auto dt = std::chrono::high_resolution_clock::now() - t0;
        write_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count();
    }

    write_time_ns = write_time_ns / cycles;
    spdlog::info("ZMQ writer terminates. Write time, ns: {:.1f}", write_time_ns);

    thread_barrier.arrive_and_wait();
}

template <typename data_type, std::size_t alignment_bytes>
void zmq_reader(zmq::context_t &ctx,
                std::size_t block_size,
                std::size_t cycles,
                std::size_t index,
                std::barrier<> &thread_barrier,
                double &read_time_ns)
{
    spdlog::info("Reader {} starts", index);

    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("inproc://endpoint");
    subscriber.set(zmq::sockopt::subscribe, "");

    aligned_array<data_type, alignment_bytes> dst(block_size);
    zmq::mutable_buffer buffer(dst.data(), dst.size() * sizeof(data_type));
    zmq::recv_buffer_result_t result{};

    thread_barrier.arrive_and_wait();

    read_time_ns = 0;
    for (size_t k = 0; k < cycles - 1; ++k)
    {
        const auto t0 = std::chrono::high_resolution_clock::now();
        result = subscriber.recv(buffer, zmq::recv_flags::none);
        const auto dt = std::chrono::high_resolution_clock::now() - t0;
        read_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count();
    }

    read_time_ns = read_time_ns / (cycles - 1);

    thread_barrier.arrive_and_wait();
    spdlog::info("Reader {} terminates. Read time, ns: {:.1f}", index, read_time_ns);
}

template <typename data_type, std::size_t alignment_bytes>
std::vector<double> run_zmq_benchmark(std::size_t block_size,
                                      std::size_t num_readers,
                                      std::size_t cycles)
{
    std::barrier<> thread_barrier(num_readers + 1);
    std::vector<double> times(num_readers + 1, 0.0);

    zmq::context_t ctx(0);

    std::thread writer_thread(zmq_writer<data_type, alignment_bytes>,
                              std::ref(ctx),
                              block_size,
                              cycles,
                              std::ref(thread_barrier),
                              std::ref(times[0]));

    std::vector<std::thread> readers;
    for (std::size_t k = 0; k < num_readers; ++k)
    {
        readers.emplace_back(zmq_reader<data_type, alignment_bytes>,
                             std::ref(ctx),
                             block_size,
                             cycles,
                             k,
                             std::ref(thread_barrier),
                             std::ref(times[k + 1]));
    }

    writer_thread.join();
    for (auto &r : readers)
    {
        r.join();
    }

    return times;
}