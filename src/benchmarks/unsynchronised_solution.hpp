#pragma once

#include "aligned_array.hpp"
#include "storage.hpp"
#include <cstring>

template <typename data_type, std::size_t alignment_bytes, typename storage>
class unsynchronised_solution
{
    const std::size_t n_blocks;
    const std::size_t b_size;
    std::size_t offset_write;
    storage a;

public:
    unsynchronised_solution(std::size_t num_blocks, std::size_t block_size)
        : n_blocks(num_blocks),
          b_size(block_size),
          offset_write(0),
          a(num_blocks * block_size)
    {
    }
    ~unsynchronised_solution() = default;

    [[nodiscard]] auto size() noexcept -> std::size_t { return n_blocks * b_size; }

    void fill(data_type value)
    {
        a.fill(value);
    }

    void write(const data_type *src, std::size_t size)
    {
        if (src != nullptr && size == b_size)
        {
            std::memcpy(a.write_offset(offset_write), src, size * sizeof(data_type));
            offset_write += size;
            offset_write = offset_write % n_blocks * b_size;
            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }

    void read(data_type *dst, std::size_t size, std::size_t offset)
    {
        if (dst != nullptr && size == b_size)
        {
            std::memcpy(dst, a.read_offset(offset), size * sizeof(data_type));
            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }
};

template <typename data_type, std::size_t alignment_bytes>
using unsync_solution = unsynchronised_solution<data_type,
                                                alignment_bytes,
                                                ring_buffer<data_type, alignment_bytes>>;

template <typename data_type, std::size_t alignment_bytes>
using memcpy_solution = unsynchronised_solution<data_type,
                                                alignment_bytes,
                                                memcpy_test_buffer<data_type, alignment_bytes>>;
