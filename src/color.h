#pragma once

#include <cstdint>

namespace minity
{

typedef u_int32_t color;

const color white{0xffffffff};
const color gray50{0x7f7f7fff};
const color black{0x000000ff};

const color red{0xff0000ff};
const color green{0x00ff00ff};
const color blue{0x0000ffff};
const color yellow{0xffff00ff};


#ifndef MINITY_COLOR_TYPES_ONLY

void printColor(color color)
{
    u_int8_t r = (u_int8_t)(color >> 24);
    u_int8_t g = (u_int8_t)(color >> 16);
    u_int8_t b = (u_int8_t)(color >> 8);
    u_int8_t a = (u_int8_t)(color >> 0);
    std::cout << "color r:" << std::to_string(r)
        << " color b:" << std::to_string(b)
        << " color g:" << std::to_string(g)
        << " color a:" << std::to_string(a) << std::endl;
}

color adjustColor(color color, float multiplier)
{
    u_int8_t r = (u_int8_t)(color >> 24);
    u_int8_t g = (u_int8_t)(color >> 16);
    u_int8_t b = (u_int8_t)(color >> 8);
    u_int8_t a = (u_int8_t)(color >> 0);

    r *= multiplier;
    g *= multiplier;
    b *= multiplier;

    return (r << 24) | (g << 16) | (b << 8) | a;
}
#endif // MINITY_COLOR_TYPES_ONLY

} // namespace minity
