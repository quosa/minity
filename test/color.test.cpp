#include <catch2/catch.hpp>
#include <iostream>

#include "color.h"

TEST_CASE("adjust color by one")
{
    u_int32_t color{0x11223344};
    REQUIRE(minity::adjustColor(color, 1.0f) == color);
}

TEST_CASE("adjust color by zero")
{
    u_int32_t color{0x11223344};
    REQUIRE(minity::adjustColor(color, 0.0f) == 0x00000044); // alpha is not touched
}
