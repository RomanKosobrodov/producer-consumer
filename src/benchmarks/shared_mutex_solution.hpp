#pragma once

#include "storage.hpp"
#include <cstring>
#include <shared_mutex>
#include <vector>

template <typename data_type, std::size_t alignment_bytes>
class shared_mutex_solution
{
    std::size_t n_blocks;
    std::size_t b_size;
    std::size_t offset_write;
    std::vector<std::shared_mutex> mus;
    aligned_array<data_type, alignment_bytes> a;

public:
    shared_mutex_solution(std::size_t num_blocks, std::size_t block_size)
        : n_blocks(num_blocks),
          b_size(block_size),
          offset_write(0),
          mus(num_blocks),
          a(std::move(aligned_array<data_type, alignment_bytes>(num_blocks * block_size)))
    {
    }
    ~shared_mutex_solution() = default;

    [[nodiscard]] auto size() noexcept -> std::size_t { return n_blocks * b_size; }

    void fill(data_type value)
    {
        fill<data_type, alignment_bytes>(a, value);
    }

    void write(const data_type *src, std::size_t size)
    {
        if (src != nullptr && size == b_size)
        {
            const size_t index = offset_write / size;
            {
                std::unique_lock lock(mus[index]);
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
                std::shared_lock lock(mus[index]);
                std::memcpy(dst, a.offset(offset), size * sizeof(data_type));
            }
            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }
};