#pragma once

#include <new>
#include <type_traits>
#include <cstring>
#include <utility>   // std::exchange
#include <exception> // std::out_of_range
#include <format>

template <typename T, std::size_t alignment_bytes>
concept ValidByteAlignment = (sizeof(T) <= alignment_bytes &&
                              alignment_bytes > 0 &&
                              (alignment_bytes & (alignment_bytes - 1)) == 0);

template <typename T, std::size_t alignment_bytes = 16>
    requires(std::is_arithmetic_v<T> && ValidByteAlignment<T, alignment_bytes>)
class aligned_array
{
    std::size_t count;
    T *ptr;

public:
    aligned_array() : count(0), ptr(nullptr){};
    aligned_array(std::size_t num_elements)
        : count(num_elements), ptr(allocate(num_elements)){};

    aligned_array(const aligned_array &other)
        : count(other.count), ptr(allocate(other.count))
    {
        std::memcpy(ptr, other.ptr, count * sizeof(T));
    }

    aligned_array(aligned_array &&other) : count(std::exchange(other.count, 0)),
                                           ptr(std::exchange(other.ptr, nullptr))
    {
    }

    aligned_array &operator=(const aligned_array &other)
    {
        if (this == &other)
        {
            return *this;
        }

        if (count != other.count)
        {
            operator delete[](ptr, alignment_bytes);
            count = other.count;
            ptr = allocate(count);
        }

        std::memcpy(ptr, other.ptr, count * sizeof(T));
    }

    aligned_array &operator=(aligned_array &&other)
    {
        if (this == &other)
        {
            return *this;
        }
        std::swap(count, other.count);
        std::swap(ptr, other.ptr);
    }

    ~aligned_array()
    {
        operator delete[](ptr, std::align_val_t{alignment_bytes}); // nullptr is a valid input for delete []
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return count; }
    [[nodiscard]] auto data() const noexcept -> T * { return ptr; }
    [[nodiscard]] auto offset(std::size_t d) const -> T *
    {
        if (d < count)
        {
            return ptr + d;
        }
        throw std::out_of_range(std::format("offset ({}) exceeds valid range [0 .. {}]", d, count - 1));
    }

private:
    [[nodiscard]] T *allocate(std::size_t num_elements) const
    {
        return new (std::align_val_t{alignment_bytes}) T[num_elements];
    }
};

template <typename T, size_t ab>
void fill(aligned_array<T, ab> &dst, T value)
{
    const size_t last = dst.size() - 1;
    std::fill(dst.offset(0), dst.offset(last) + 1, value);
}