#include "benchmark.hpp"
#include "unsynchronised_solution.hpp"
#include "synchronised_solution.hpp"
#include "seqlock_solution.hpp"
#include "zmq_benchmark.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/fmt.h>

#include <string>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <algorithm>
#include <fstream>

struct parameters
{
    std::size_t num_blocks{10};
    std::size_t block_size{16};
    std::size_t num_readers{3};
    std::size_t num_cycles{1000000};
    bool enable_memcpy{true};
    bool enable_seqlock{true};
    bool enable_shared_lock{true};
    bool enable_mutex_lock{true};
    bool enable_zmq{true};
};

inline std::string print_results(const std::string &message, const parameters &params, std::vector<double> &times, const char separator = ' ')
{
    std::string s = fmt::format("{}", "{\n");
    s += fmt::format("\"implementation\": \"{}\",\n", message);
    s += fmt::format("\"num_cycles\": \"{}\",\n", params.num_cycles);
    s += fmt::format("\"block_size\": \"{}\",\n", params.block_size);
    s += fmt::format("\"num_blocks\": \"{}\",\n", params.num_blocks);
    s += fmt::format("\"num_readers\": \"{}\",\n", params.num_readers);
    s += fmt::format("\"writer\": {:.1f},\n", times[0]);
    s += fmt::format("\"readers\": [");
    std::vector<double> sorted(std::begin(times) + 1, std::end(times));
    std::sort(std::begin(sorted), std::end(sorted));
    for (auto k = 0; k < sorted.size() - 1; ++k)
    {
        s += fmt::format("{:.1f}, ", sorted[k]);
    }
    s += fmt::format("{:.1f}]\n", sorted[sorted.size() - 1]);
    return s + fmt::format("{}{}\n", "}", separator);
}

std::string run_benchmark(const parameters &p)
{
    std::string s;

    constexpr std::size_t alignment_bytes{16};
    using data_type = std::uint64_t;

    std::vector<double> results;

    if (p.enable_memcpy)
    {
        using memcpy_solution_type = memcpy_solution<data_type, alignment_bytes>;
        results = run_benchmark<memcpy_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                                  p.block_size,
                                                                                  p.num_readers,
                                                                                  p.num_cycles);
        s += print_results("Memcpy", p, results, ',');
    }

    if (p.enable_seqlock)
    {
        using seqlock_solution_type = seqlock_solution<data_type, alignment_bytes>;
        results = run_benchmark<seqlock_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                                   p.block_size,
                                                                                   p.num_readers,
                                                                                   p.num_cycles);
        s += print_results("SeqLock", p, results, ',');
    }

    if (p.enable_shared_lock)
    {
        using shared_solution_type = shared_solution<data_type, alignment_bytes>;
        results = run_benchmark<shared_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                                  p.block_size,
                                                                                  p.num_readers,
                                                                                  p.num_cycles);
        s += print_results("Shared mutex", p, results, ',');
    }

    if (p.enable_mutex_lock)
    {
        using exclusive_solution_type = exclusive_solution<data_type, alignment_bytes>;
        results = run_benchmark<exclusive_solution_type, data_type, alignment_bytes>(p.num_blocks,
                                                                                     p.block_size,
                                                                                     p.num_readers,
                                                                                     p.num_cycles);
        s += print_results("Mutex", p, results, ',');
    }

    if (p.enable_zmq)
    {
        parameters p_zmq = p;
        p_zmq.num_cycles = 100;
        results = run_zmq_benchmark<data_type, alignment_bytes>(p_zmq.block_size,
                                                                p_zmq.num_readers,
                                                                p_zmq.num_cycles);

        s += print_results("ZMQ", p, results, ',');
    }

    return s;
}

int main(int argc, char *argv[])
{
    std::shared_ptr<spdlog::logger> file_logger = spdlog::rotating_logger_mt("benchmark", "benchmark.log", 1048576 * 5, 3);
    file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e %z] [%-8t] [%-8l] %v");
    file_logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(file_logger);

    constexpr std::size_t block_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
    constexpr std::size_t readers[] = {1, 2, 3, 4, 5};
    parameters p;
    std::string s = "{ \"results\": [\n";

    for (const auto &b : block_sizes)
    {
        for (const auto &r : readers)
        {
            p.block_size = b;
            p.num_readers = r;
            s += run_benchmark(p);
        }
    }
    s[s.size() - 2] = ' ';
    s += "]\n}";

    std::ofstream fo("results.json");
    fo << s << "\n";

    file_logger->flush();
    spdlog::shutdown();
    return 0;
}