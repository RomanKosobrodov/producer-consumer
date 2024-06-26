#pragma once

#include "storage.hpp"
#include <cstring>
#include <shared_mutex>
#include <vector>

template <std::size_t false_sharing_range = 128>
    requires(false_sharing_range > sizeof(std::atomic<std::size_t>))
struct cursor
{
    alignas(false_sharing_range) std::atomic<std::size_t> seq;
    char padding_[(false_sharing_range - sizeof(seq)) % false_sharing_range];
};

template <typename data_type, std::size_t alignment_bytes>
class seqlock_solution
{
    std::size_t n_blocks;
    std::size_t b_size;
    std::vector<cursor<>> cursors;
    std::size_t offset_write;
    aligned_array<data_type, alignment_bytes> a;

public:
    seqlock_solution(std::size_t num_blocks, std::size_t block_size)
        : n_blocks(num_blocks),
          b_size(block_size),
          cursors(num_blocks),
          offset_write(0),
          a(num_blocks * block_size)
    {
    }
    ~seqlock_solution() = default;

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
            std::size_t seq0 = cursors[index].seq.load(std::memory_order_relaxed);
            cursors[index].seq.store(seq0 + 1, std::memory_order_release);
            std::atomic_signal_fence(std::memory_order_acq_rel);
            std::memcpy(a.offset(offset_write), src, size * sizeof(data_type));
            std::atomic_signal_fence(std::memory_order_acq_rel);
            cursors[index].seq.store(seq0 + 2, std::memory_order_release);
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
            std::size_t seq0;
            std::size_t seq1;
            do
            {
                seq0 = cursors[index].seq.load(std::memory_order_acquire);
                std::atomic_signal_fence(std::memory_order_acq_rel);
                std::memcpy(dst, a.offset(offset), size * sizeof(data_type));
                std::atomic_signal_fence(std::memory_order_acq_rel);
                seq1 = cursors[index].seq.load(std::memory_order_acquire);
            } while (seq0 != seq1 || seq0 & 1);

            return;
        }
        throw std::runtime_error("invalid pointer or block size");
    }
};