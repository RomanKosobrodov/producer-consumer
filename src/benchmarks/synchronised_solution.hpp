#pragma once

#include "aligned_array.hpp"
#include <cstring>
#include <vector>
#include <mutex>
#include <shared_mutex>

template <typename data_type, std::size_t alignment_bytes, class mutex_class, class write_lock, class read_lock>
class synchronised_solution
{
    std::size_t n_blocks;
    std::size_t b_size;
    std::size_t offset_write;
    std::vector<mutex_class> mus;
    aligned_array<data_type, alignment_bytes> a;

public:
    synchronised_solution(std::size_t num_blocks, std::size_t block_size)
        : n_blocks(num_blocks),
          b_size(block_size),
          offset_write(0),
          mus(num_blocks),
          a(num_blocks * block_size)
    {
    }
    ~synchronised_solution() = default;

    [[nodiscard]] auto size() noexcept -> std::size_t { return n_blocks * b_size; }

    void fill(data_type value)
    {
        fill_array(a, value);
    }

    void write(const data_type *src, std::size_t size)
    {
        if (src != nullptr && size == b_size)
        {
            const size_t index = offset_write / size;
            {
                const write_lock lock(mus[index]);
                std::memcpy(a.offset(offset_write), src, size * sizeof(data_type));
            }
            offset_write += size;
            offset_write = offset_write % a.size();
            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }

    void read(data_type *dst, std::size_t size, std::size_t offset)
    {
        if (dst != nullptr && size == b_size)
        {
            const size_t index = offset / size;
            {
                const read_lock lock(mus[index]);
                std::memcpy(dst, a.offset(offset), size * sizeof(data_type));
            }
            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }
};

template <typename data_type, std::size_t alignment_bytes>
using shared_solution = synchronised_solution<data_type,
                                              alignment_bytes,
                                              std::shared_mutex,
                                              std::unique_lock<std::shared_mutex>,
                                              std::shared_lock<std::shared_mutex>>;

template <typename data_type, std::size_t alignment_bytes>
using exclusive_solution = synchronised_solution<data_type,
                                                 alignment_bytes,
                                                 std::mutex,
                                                 std::lock_guard<std::mutex>,
                                                 std::lock_guard<std::mutex>>;