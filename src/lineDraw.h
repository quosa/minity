#include <algorithm>
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

// #include "simpleMath.h"
// #include "sdlHelpers.h"

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
    if (depth < g_DepthBuffer[y * g_SDLWidth + x] || true)
    {
        g_SDLBackBuffer[y * g_SDLWidth + x] = rgba_color;
        g_DepthBuffer[y * g_SDLWidth + x] = depth;
    } else {
        std::cout << "Z: " << depth << " < " << g_DepthBuffer[y * g_SDLWidth + x] << "?" << std::endl;
    }
}

void plotLineLow(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    float D = 2 * dy - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            y += yi;
            D -= 2 * dx;
        }
        D += 2 * dy;
    }
}

void plotLineHigh(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    float D = 2 * dx - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++)
    {
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            x += xi;
            D -= 2 * dy;
        }
        D += 2 * dx;
    }
}

// todo: convert to also calculate the z-coordinate:
//      void plotLine(point from, point to, u_int32_t rgba_color)
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void plotLine(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
{
    // std::cout << "plotline: from (" << std::to_string(x0) << ", "
    //     << std::to_string(y0) << ") to ("
    //     << std::to_string(x1) << ", "
    //     << std::to_string(y1) << ")" << std::endl;
    if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
            plotLineLow(x1, y1, x0, y0, depth, rgba_color);
        else
            plotLineLow(x0, y0, x1, y1, depth, rgba_color);
    }
    else
    {
        if (y0 > y1)
            plotLineHigh(x1, y1, x0, y0, depth, rgba_color);
        else
            plotLineHigh(x0, y0, x1, y1, depth, rgba_color);
    }
}

// convert to use point screenXY(vec3 vertice)
// and remove average depth to get proper z-buffer
void drawLine(vec3 from, vec3 to, u_int32_t rgba_color)
{
    int ax, ay, bx, by;

    // if (from.z > 0 || to.z > 0)
    // {
    //     std::cerr << "Z CLIPPING!!!" << std::endl;
    //     return;
    // }

    if (from.x < -1 || from.x > 1 || from.y < -1 || from.y > 1 || to.x < -1 || to.x > 1 || to.y < -1 || to.y > 1)
    {
        std::cerr << "XY CLIPPING!!!" << std::endl;
        std::cerr << "from: " << from << " to: " << to << std::endl;
        return;
    }

    // todo: calculate per pixel depth!
    float depth = (from.z + to.z) / 2.0f;
    // std::cout << "line depth is " << std::to_string(depth) << std::endl;

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    ax = (int)(((from.x + 1.0f) / 2.0f) * (float)g_SDLWidth);
    ay = (int)((1.0f - ((from.y + 1.0f) / 2.0f)) * (float)g_SDLHeight);
    bx = (int)(((to.x + 1.0f) / 2.0f) * (float)g_SDLWidth);
    by = (int)((1.0f - ((to.y + 1.0f) / 2.0f)) * (float)g_SDLHeight);
    plotLine(ax, ay, bx, by, depth, rgba_color);
}
