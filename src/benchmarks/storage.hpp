#pragma once
#include "aligned_array.hpp"

/// Helper classes to implement thread-unsafe and safe mock-up versions of unsynchronised memory buffer

/// @brief  Thread-unsafe ring buffer. Reading and writing from different threads causes UB unless external synchronisation is used
/// @tparam data_type type of stored data
/// @tparam alignment_bytes alignment in bytes of the underlying C-style array
template <typename data_type, std::size_t alignment_bytes>
class ring_buffer
{
    aligned_array<data_type, alignment_bytes> a;

public:
    ring_buffer(std::size_t num_elements)
        : a(num_elements)
    {
    }

    void fill(data_type value)
    {
        fill_array(a, value);
    }

    [[nodiscard]] auto write_offset(std::size_t d) const -> data_type *
    {
        return a.offset(d);
    }

    [[nodiscard]] auto read_offset(std::size_t d) const -> data_type *
    {
        return a.offset(d);
    }
};

/// @brief  Mock-up thread safe storage to time reads and writes without causing UB (using one buffer for reading and another for writing)
/// @tparam data_type type of stored data
/// @tparam alignment_bytes alignment in bytes of the underlying C-style array
template <typename data_type, std::size_t alignment_bytes>
class memcpy_test_buffer
{
    aligned_array<data_type, alignment_bytes> a;
    aligned_array<data_type, alignment_bytes> b;

public:
    memcpy_test_buffer(std::size_t num_elements)
        : a(num_elements),
          b(num_elements)
    {
        fill_array(b, data_type{});
    }

    void fill(data_type value)
    {
        fill_array(a, value);
    }

    [[nodiscard]] auto write_offset(std::size_t d) const -> data_type *
    {
        return a.offset(d);
    }

    [[nodiscard]] auto read_offset(std::size_t d) const -> data_type *
    {
        return b.offset(d);
    }
};