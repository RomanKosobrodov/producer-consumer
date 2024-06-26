#include "benchmark.hpp"
#include "bad_solution.hpp"
#include "shared_mutex_solution.hpp"
#include "seqlock_solution.hpp"
#include "zmq_benchmark.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/fmt.h>
#include <format>
#include <vector>

void print_results(const std::string &message, const std::vector<double> &times)
{
    fmt::print("{}", message);
    fmt::print("Write time,  ns: {:.1f}\n", times[0]);
    for (auto k = 1; k < times.size(); ++k)
    {
        fmt::print("Read {} time, ns: {:.1f}\n", k - 1, times[k]);
    }
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
    using bad_solution_type = bad_solution<data_type, alignment_bytes>;
    results = run_benchmark<bad_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                           block_size,
                                                                           num_readers,
                                                                           num_cycles);
    print_results("\nBad solution\n", results);

    using shared_solution_type = shared_mutex_solution<data_type, alignment_bytes>;
    results = run_benchmark<shared_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                              block_size,
                                                                              num_readers,
                                                                              num_cycles);
    print_results("\nShared mutex solution\n", results);

    using seqlock_solution_type = seqlock_solution<data_type, alignment_bytes>;
    results = run_benchmark<seqlock_solution_type, data_type, alignment_bytes>(num_blocks,
                                                                               block_size,
                                                                               num_readers,
                                                                               num_cycles);
    print_results("\nSeqLock solution\n", results);

    constexpr std::size_t num_zmq_cycles{100};
    results = run_zmq_benchmark<data_type, alignment_bytes>(block_size,
                                                            num_readers,
                                                            num_zmq_cycles);

    print_results("\nZMQ solution\n", results);

    file_logger->flush();
    spdlog::shutdown();
    return 0;
}