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

void print_results(const std::string &message, const std::vector<double> &times)
{
    fmt::print("\"{}\", {:.1f}", message, times[0]);
    for (auto k = 1; k < times.size(); ++k)
    {
        fmt::print(", {:.1f}", times[k]);
    }
    fmt::print("\n");
}

int main(int argc, char *argv[])
{
    std::shared_ptr<spdlog::logger> file_logger = spdlog::rotating_logger_mt("benchmark", "benchmark.log", 1048576 * 5, 3);
    file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e %z] [%-8t] [%-8l] %v");
    file_logger->set_level(spdlog::level::debug);

    spdlog::set_default_logger(file_logger);

    constexpr std::size_t num_blocks{10};
    constexpr std::size_t block_size{16};
    constexpr std::size_t alignment_bytes{16};
    constexpr std::size_t num_readers{3};
    constexpr std::size_t num_cycles{1000000};

    using data_type = std::uint64_t;
    std::vector<double> results;

    using memcpy_solution_type = memcpy_solution<data_type, alignment_bytes>;
    results = run_benchmark<memcpy_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                              block_size,
                                                                              num_readers,
                                                                              num_cycles);
    print_results("Memcpy", results);

    using unsync_solution_type = unsync_solution<data_type, alignment_bytes>;
    results = run_benchmark<unsync_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                              block_size,
                                                                              num_readers,
                                                                              num_cycles);
    print_results("Unsync", results);

    using seqlock_solution_type = seqlock_solution<data_type, alignment_bytes>;
    results = run_benchmark<seqlock_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                               block_size,
                                                                               num_readers,
                                                                               num_cycles);
    print_results("SeqLock", results);

    using shared_solution_type = shared_solution<data_type, alignment_bytes>;
    results = run_benchmark<shared_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                              block_size,
                                                                              num_readers,
                                                                              num_cycles);
    print_results("Shared mutex", results);

    using exclusive_solution_type = exclusive_solution<data_type, alignment_bytes>;
    results = run_benchmark<exclusive_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                                 block_size,
                                                                                 num_readers,
                                                                                 num_cycles);
    print_results("Mutex", results);

    constexpr std::size_t num_zmq_cycles{100};
    results = run_zmq_benchmark<data_type, alignment_bytes>(block_size,
                                                            num_readers,
                                                            num_zmq_cycles);

    print_results("ZMQ", results);

    file_logger->flush();
    spdlog::shutdown();
    return 0;
}