#include <algorithm>
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#define MATH_TYPES_ONLY
#include "simpleMath.h"

// forward declaration
void plotLine(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color);
void setPixel(int x, int y, float depth, u_int32_t rgba_color);

void plotHorizontalLine(point start, point end, u_int32_t rgba_color)
{
    assert(start.y == end.y); // line needs to be horizontal

    // clipping #1 to viewport/projection space
    if (start.y < 0 || start.y >= g_SDLHeight)
        return; // out of screen bounds

    auto y = start.y;
    auto sx = start.x > end.x ? end.x : start.x;
    auto ex = start.x > end.x ? start.x : end.x;
    auto sz = start.x > end.x ? end.z : start.z;
    auto ez = start.x > end.x ? start.z : end.z;
    auto dz = (ez - sz) / (ex - sx);

    assert(ex >= sx);

    // clipping #2 to viewport/projection space
    sx = std::max(0, sx);
    sx = std::min(sx, g_SDLWidth);
    ex = std::max(0, ex);
    ex = std::min(ex, g_SDLWidth);

    // draw horizontal line and use the currect z value for each pixel
    float z = sz;
    for (int x = sx; x <= ex; x++)
    {
        setPixel(x, y, z, rgba_color);
        z += dz;
    }
}

void setPixel(int x, int y, float depth, u_int32_t rgba_color)
{
    // clipping #3 to viewport/projection space
    if (x < 0 || x >= g_SDLWidth || y < 0 || y >= g_SDLHeight)
        return;

    // clipping #4 don't draw pixes that are behind the camera
    // (in view space, the camera is at origin looking towards Z+)
    if (depth < 0.0f)
        return;

    if (depth > 100.0f)
    {
        std::cout << "Pixel far away? (" << x << "," << y << "," << std::to_string(depth) << ")" << std::endl;
    }

    // Z-buffer check: discard deeper pixels (further away from the camera)
    if (depth < g_DepthBuffer[y * g_SDLWidth + x])
    {
        g_SDLBackBuffer[y * g_SDLWidth + x] = rgba_color;
        g_DepthBuffer[y * g_SDLWidth + x] = depth;
    }
}

point screenXY(vec3 vertice)
{
    // vertice must be in normalized device coordinates (NDC)
    // (we choose opengl style: x, y, z inside [-1,1] - Direct3D has z [0,1])

    // screen coordinates are (0, 0) top-left
    // and (width, height) bottom-right

    assert(vertice.x >= -1 && vertice.x <= 1
        && vertice.y >= -1 && vertice.y <= 1
        && vertice.z >= -1 && vertice.z <= 1);

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    // z=-1 -> 0 and z=1 -> 1
    return point{
        (int)(((vertice.x + 1.0f) / 2.0f) * (float)g_SDLWidth),
        (int)((1.0f - ((vertice.y + 1.0f) / 2.0f)) * (float)g_SDLHeight),
        (vertice.z + 1.0f) / 2.0f};
};

void fillTopFlatTriangle(point v1, point v2, point v3, u_int32_t rgba_color)
{
    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillTopFlatTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    float dx31 = 1.0 * (v3.x - v1.x) / (v3.y - v1.y);
    float dx32 = 1.0 * (v3.x - v2.x) / (v3.y - v2.y);
    float sx = 1.0 * v3.x;
    float ex = 1.0 * v3.x;

    float dz31 = 1.0 * (v3.z - v1.z) / (v3.y - v1.y);
    float dz32 = 1.0 * (v3.z - v2.z) / (v3.y - v2.y);
    float sz = v3.z;
    float ez = v3.z;

    // std::cout << "fTFT: dx31 " << std::to_string(dx31) << ", dx32 " << std::to_string(dx32) << std::endl;
    for (int y = v3.y; y > v2.y; y--)
    {
        plotHorizontalLine(
            point{(int)sx, y, sz},
            point{(int)ex, y, ez},
            rgba_color);

        sx -= dx31;
        ex -= dx32;
        sz -= dz31;
        ez -= dz32;
    }
};

void fillBottomFlatTriangle(point v1, point v2, point v3, u_int32_t rgba_color)
{

    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillBottomFlatTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    float dx21 = 1.0 * (v2.x - v1.x) / (v2.y - v1.y);
    float dx31 = 1.0 * (v3.x - v1.x) / (v3.y - v1.y);
    float sx = 1.0 * v1.x;
    float ex = 1.0 * v1.x;

    float dz21 = 1.0 * (v2.z - v1.z) / (v2.y - v1.y);
    float dz31 = 1.0 * (v3.z - v1.z) / (v3.y - v1.y);
    float sz = 1.0 * v1.z;
    float ez = 1.0 * v1.z;

    // std::cout << "fBFT: dx21 " << std::to_string(dx21) << ", dx31 " << std::to_string(dx31) << std::endl;

    for (int y = v1.y; y <= v2.y; y++)
    {
        plotHorizontalLine(
            point{(int)sx, y, sz},
            point{(int)ex, y, ez},
            rgba_color);
        sx += dx21;
        ex += dx31;
        sz += dz21;
        ez += dz31;
    }
};

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillTriangle(vec3 vertices[3], u_int32_t rgba_color)
{
    point v1 = screenXY(vertices[0]);
    point v2 = screenXY(vertices[1]);
    point v3 = screenXY(vertices[2]);

    if (v1.y > v3.y)
        pSwap(&v1, &v3);
    if (v1.y > v2.y)
        pSwap(&v1, &v2);
    if (v2.y > v3.y)
        pSwap(&v2, &v3);

    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    if (v2.y == v3.y)
    {
        fillBottomFlatTriangle(v1, v2, v3, rgba_color);
    }
    else if (v1.y == v2.y)
    {
        fillTopFlatTriangle(v1, v2, v3, rgba_color);
    }
    else
    {
        // general case, add intermediate point to have
        // one top and bottom flat triangle
        point v4{
            (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)),
            v2.y,
            v1.z + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.z - v1.z)};
        fillBottomFlatTriangle(v1, v2, v4, rgba_color);
        fillTopFlatTriangle(v2, v4, v3, rgba_color);
    }
}
