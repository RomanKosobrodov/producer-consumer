#include <catch2/catch_test_macros.hpp>
#include <aligned_array.hpp>

TEST_CASE("aligned_array can be default constructed")
{
    aligned_array<int> a;
    REQUIRE(a.size() == 0);
}

TEST_CASE("aligned_array is indeed aligned")
{
    const std::size_t n = 2;
    constexpr std::size_t alignment_bytes = 32;
    aligned_array<int, alignment_bytes> a(n);
    REQUIRE(reinterpret_cast<uintptr_t>(a.data()) % alignment_bytes == 0);
}

TEST_CASE("aligned_array.offset checks bounds")
{
    constexpr std::size_t n = 10;
    aligned_array<double> a(n);
    double *p;
    REQUIRE_NOTHROW(p = a.offset(0));
    REQUIRE_NOTHROW(p = a.offset(n - 1));
    REQUIRE_THROWS_AS(p = a.offset(n), std::out_of_range);
}