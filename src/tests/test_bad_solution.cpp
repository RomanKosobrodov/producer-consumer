#include <catch2/catch_test_macros.hpp>
#include <bad_solution.hpp>

TEST_CASE("bad_solution is correctly implemented")
{
    constexpr std::size_t num_blocks = 10;
    constexpr std::size_t block_size = 640;
    bad_solution<std::uint64_t> a(num_blocks, block_size);
    aligned_array<std::uint64_t> src(block_size);
    SECTION("write catches wrong input")
    {
        REQUIRE_THROWS_AS(a.write(nullptr, block_size), std::runtime_error);
        REQUIRE_THROWS_AS(a.write(src.data(), 2), std::runtime_error);
    }

    SECTION("read catches wrong input")
    {
        REQUIRE_THROWS_AS(a.read(nullptr, block_size), std::runtime_error);
        REQUIRE_THROWS_AS(a.read(src.data(), 2), std::runtime_error);
    }

    SECTION("writes and reads data")
    {
        constexpr std::size_t count = 100;
        aligned_array<std::uint64_t> dst(block_size);
        for (size_t k = 0; k < count; ++k)
        {
            a.write(src.data(), block_size);
            a.read(dst.data(), block_size);
            REQUIRE(std::memcmp(src.data(), dst.data(), block_size * sizeof(std::uint64_t)) == 0);
        }
    }
}