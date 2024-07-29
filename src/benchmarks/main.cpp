#include "benchmark.hpp"
#include "unsynchronised_solution.hpp"
#include "synchronised_solution.hpp"
#include "seqlock_solution.hpp"
#include "zmq_benchmark.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/fmt.h>

#include <shared_mutex>
#include <mutex>
#include <format>
#include <vector>
#include <algorithm>

struct parameters
{
    std::size_t num_blocks{10};
    std::size_t block_size{16};
    std::size_t num_readers{3};
    std::size_t num_cycles{1000000};
};

void print_results(const std::string &message, const parameters &params, std::vector<double> &times, const char separator = ' ')
{
    fmt::print("{}", "{\n");
    fmt::print("\"implementation\": \"{}\",\n", message);
    fmt::print("\"num_cycles\": \"{}\",\n", params.num_cycles);
    fmt::print("\"block_size\": \"{}\",\n", params.block_size);
    fmt::print("\"num_blocks\": \"{}\",\n", params.num_blocks);
    fmt::print("\"num_readers\": \"{}\",\n", params.num_readers);
    fmt::print("\"reader\": {:.1f},\n", times[0]);
    fmt::print("\"writers\": [");
    std::vector<double> sorted(std::begin(times) + 1, std::end(times));
    std::sort(std::begin(sorted), std::end(sorted));
    for (auto k = 0; k < sorted.size() - 1; ++k)
    {
        fmt::print("{:.1f}, ", sorted[k]);
    }
    fmt::print("{:.1f}]\n", sorted[sorted.size() - 1]);
    fmt::print("{}{}\n", "}", separator);
}

void run_benchmark(const parameters &p)
{
    constexpr std::size_t alignment_bytes{16};
    using data_type = std::uint64_t;

    std::vector<double> results;

    using memcpy_solution_type = memcpy_solution<data_type, alignment_bytes>;
    results = run_benchmark<memcpy_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                              p.block_size,
                                                                              p.num_readers,
                                                                              p.num_cycles);
    print_results("Memcpy", p, results, ',');

    using unsync_solution_type = unsync_solution<data_type, alignment_bytes>;
    results = run_benchmark<unsync_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                              p.block_size,
                                                                              p.num_readers,
                                                                              p.num_cycles);
    print_results("Unsync", p, results, ',');

    using seqlock_solution_type = seqlock_solution<data_type, alignment_bytes>;
    results = run_benchmark<seqlock_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                               p.block_size,
                                                                               p.num_readers,
                                                                               p.num_cycles);
    print_results("SeqLock", p, results, ',');

    using shared_solution_type = shared_solution<data_type, alignment_bytes>;
    results = run_benchmark<shared_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                              p.block_size,
                                                                              p.num_readers,
                                                                              p.num_cycles);
    print_results("Shared mutex", p, results, ',');

    using exclusive_solution_type = exclusive_solution<data_type, alignment_bytes>;
    results = run_benchmark<exclusive_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                                 p.block_size,
                                                                                 p.num_readers,
                                                                                 p.num_cycles);
    print_results("Mutex", p, results, ',');

    parameters p_zmq = p;
    p_zmq.num_cycles = 100;
    results = run_zmq_benchmark<data_type, alignment_bytes>(p_zmq.block_size,
                                                            p_zmq.num_readers,
                                                            p_zmq.num_cycles);

    print_results("ZMQ", p, results);
}

int main(int argc, char *argv[])
{
    std::shared_ptr<spdlog::logger> file_logger = spdlog::rotating_logger_mt("benchmark", "benchmark.log", 1048576 * 5, 3);
    file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e %z] [%-8t] [%-8l] %v");
    file_logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(file_logger);

    parameters p;
    run_benchmark(p);

    file_logger->flush();
    spdlog::shutdown();
    return 0;
}