#pragma once

#include <limits>

#define MINITY_SCENE_TYPES_ONLY
#include "scene.h"
#include "simpleMath.h"

namespace minity
{

typedef uint32_t color;
const color black{0x000000ff};
const color white{0xffffffff};
const color red{0xff0000ff};
const color green{0x00ff00ff};
const color blue{0x0000ffff};
const color yellow{0xffff00ff};

class rasterizer
{
public:
    rasterizer(unsigned int width, unsigned int height)
        : viewportWidth(width), viewportHeight(height)
    {
        // both are addressed [x + y*viewportWidth]
        frameBuffer = std::vector<color>(viewportWidth * viewportHeight, black);
        depthBuffer = std::vector<float>(viewportWidth * viewportHeight, std::numeric_limits<float>::infinity());
    };
    void drawTriangle(const vec3 (&vertices)[3], const color rgba_color);
    void drawLine(const vec3 &from, const vec3 &to, const color rgba_color);
    void drawPoint(const vec3 &point, const color rgba_color);
    color *getFramebuffer();
    float *getDepthbuffer();
    unsigned int getViewportWidth();
    unsigned int getViewportHeight();

private:
    unsigned int viewportWidth{0};
    unsigned int viewportHeight{0};
    std::vector<color> frameBuffer;
    std::vector<float> depthBuffer;
    rasterizer(); // hide default constructor so that we get width and height
};

// fwd decl
void plotLine(const vec3 &from, const vec3 &to, const color rgba_color, rasterizer *rasterizer);
void plotTriangle(const vec3 (&vertices)[3], const color rgba_color, rasterizer *rasterizer);

// method implementations
void rasterizer::drawTriangle(const vec3 (&vertices)[3], const color rgba_color)
{
    // (void)vertices;
    // (void)rgba_color;
    std::cout << "drawTriangle: " << vertices[0] << " " << vertices[1] << " " << vertices[2] << std::endl;
    plotTriangle(vertices, rgba_color, this);
};
void rasterizer::drawLine(const vec3 &from, const vec3 &to, const color rgba_color)
{
    // clipping #3 to viewport/projection space
    std::cout << "drawLine: " << from << "->" << to << std::endl;
    if (from.x < 0 || from.x >= viewportWidth || from.y < 0 || from.y >= viewportHeight
        || to.x < 0 || to.x >= viewportWidth || to.y < 0 || to.y >= viewportHeight)
    {
        // stats.xyClipped++;
        return; // we're outside the viewport
    }

    plotLine(from, to, rgba_color, this);
};
void rasterizer::drawPoint(const vec3 &point, color rgba_color)
{
    // assert(0 <= point.x && point.x <= viewportWidth);
    // assert(0 <= point.y && point.y <= viewportHeight);

    std::cout << "drawPoint: " << point;
    // clipping #3 to viewport/projection space
    if (point.x < 0 || point.x >= viewportWidth || point.y < 0 || point.y >= viewportHeight)
    {
        // stats.xyClipped++;
        std::cout << " clipped" << std::endl;
        return; // we're outside the viewport
    }

    // z-check
    if (point.z <= depthBuffer[point.x + point.y * viewportWidth])
    {
        frameBuffer[point.x + point.y * viewportWidth] = rgba_color;
        depthBuffer[point.x + point.y * viewportWidth] = point.z;
        std::cout << " drawn" << std::endl;
    }
    else
    {
        std::cout << " z-clipped" << std::endl;
    }
};
color *rasterizer::getFramebuffer() { return (color *)frameBuffer.data(); };
float *rasterizer::getDepthbuffer() { return (float *)depthBuffer.data(); };
unsigned int rasterizer::getViewportWidth() { return viewportWidth; };
unsigned int rasterizer::getViewportHeight() { return viewportHeight; };

/**
 * @brief draw line to framebuffer
 *
 * @param from vec3 in screen coordinates
 * @param to vec3 in screen coordinates
 * @param rgba_color minity::color with 0xrrggbbaa format
 * @param rasterizer pointer to the rasterizer instance to use (facilitate easier testing)
 *
 * @see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * @see https://www.geeksforgeeks.org/bresenhams-algorithm-for-3-d-line-drawing/
 */
void plotLine(const vec3 &from, const vec3 &to, const color rgba_color, rasterizer *rasterizer)
{
    assert(rasterizer);

    int dx = abs(to.x - from.x);
    int dy = abs(to.y - from.y);
    int dz = abs(to.z - from.z);
    int xs = (to.x > from.x) ? 1 : -1;
    int ys = (to.y > from.y) ? 1 : -1;
    int zs = (to.z > from.z) ? 1 : -1;

    int x = from.x;
    int y = from.y;
    int z = from.z;
    rasterizer->drawPoint(vec3{(float)x, (float)y, (float)z}, rgba_color);

    if (dx >= dy && dx >= dz) // X-axis drives
    {
        int p1 = 2 * dy - dx;
        int p2 = 2 * dz - dx;
        while (x != (int)to.x)
        {
            x += xs;
            if (p1 >= 0)
            {
                y += ys;
                p1 -= 2 * dx;
            }
            if (p2 >= 0)
            {
                z += zs;
                p2 -= 2 * dx;
            }
            p1 += 2 * dy;
            p2 += 2 * dz;
            rasterizer->drawPoint(vec3{(float)x, (float)y, (float)z}, rgba_color);
        }
    }
    else if (dy >= dx && dy >= dz) // Y-axis drives
    {
        int p1 = 2 * dx - dy;
        int p2 = 2 * dz - dy;
        while (y != (int)to.y)
        {
            y += ys;
            if (p1 >= 0)
            {
                x += xs;
                p1 -= 2 * dy;
            }
            if (p2 >= 0)
            {
                z += zs;
                p2 -= 2 * dy;
            }
            p1 += 2 * dx;
            p2 += 2 * dz;
            rasterizer->drawPoint(vec3{(float)x, (float)y, (float)z}, rgba_color);
        }
    }
    else // Z-axis drives
    {
        int p1 = 2 * dy - dz;
        int p2 = 2 * dx - dz;
        while (z != (int)to.z)
        {
            z += zs;
            if (p1 >= 0)
            {
                y += ys;
                p1 -= 2 * dz;
            }
            if (p2 >= 0)
            {
                x += xs;
                p2 -= 2 * dz;
            }
            p1 += 2 * dy;
            p2 += 2 * dx;
            rasterizer->drawPoint(vec3{(float)x, (float)y, (float)z}, rgba_color);
        }
    }
}

struct barycentricCoordinates
{
public:
    // pre-calculate everything that we can before
    // for x, y hot-path loop
    void prepare(const vec3 face[3])
    {
        v0 = v3Sub(face[1], face[0]); // b - a
        v1 = v3Sub(face[2], face[0]); // c - a;
        d00 = v3DotProduct(v0, v0);
        d01 = v3DotProduct(v0, v1);
        d11 = v3DotProduct(v1, v1);
        invDenom = 1.0f / (d00 * d11 - d01 * d01);
    }

    // calculate only the missing elements for each x, y
    // and return the corresponding barycentric coordinates
    // TODO: change out parameters to return a vec3 with the coordinates
    //       will be used as barycentric.x etc.
    void barycentricCoordinatesAt(const vec3 face[3], const vec3 &point, float &u, float &v, float &w)
    {
        assert(invDenom != 0.0f);
        v2 = v3Sub(point, face[0]); // p - a;
        d20 = v3DotProduct(v2, v0);
        d21 = v3DotProduct(v2, v1);
        v = (d11 * d20 - d01 * d21) * invDenom;
        w = (d00 * d21 - d01 * d20) * invDenom;
        u = 1.0f - v - w;
    }

private:
    vec3 v0{0};
    vec3 v1{0};
    vec3 v2{0};
    float d00{0};
    float d01{0};
    float d11{0};
    float d20{0};
    float d21{0};
    float invDenom;
};

/**
 * @brief draw triangle to framebuffer
 *
 * @param vertices vec3[3] of triangle vertices in screen space
 * @param rgba_color minity::color with 0xrrggbbaa format
 * @param rasterizer pointer to the rasterizer instance to use (facilitate easier testing)
 */
void plotTriangle(const vec3 (&vertices)[3], const color rgba_color, rasterizer *rasterizer)
{
    const vec3 &v1 = vertices[0];
    const vec3 &v2 = vertices[1];
    const vec3 &v3 = vertices[2];

    (void)vertices;
    (void)rgba_color;
    rasterizer->getViewportHeight();
    int maxX = std::max(v1.x, std::max(v2.x, v3.x));
    int minX = std::min(v1.x, std::min(v2.x, v3.x));
    int maxY = std::max(v1.y, std::max(v2.y, v3.y));
    int minY = std::min(v1.y, std::min(v2.y, v3.y));

    // barycentric coordinates
    float u{0};
    float v{0};
    float w{0};
    barycentricCoordinates bc{};

    // minor optimization: calculate
    // the constant parts only once
    bc.prepare(vertices);

    const int width = rasterizer->getViewportWidth();
    const int height = rasterizer->getViewportHeight();
    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {

            // TODO: many implementations move the inspection point to center of pixel
            //       need to compare the output and see if this helps with tears etc.
            // vec3 point  = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, 0};
            vec3 point = {static_cast<float>(x), static_cast<float>(y), 0}; // IS THIS 0 CORRECT ???
            bc.barycentricCoordinatesAt(vertices, point, u, v, w);

            if (u < 0 || v < 0 || w < 0)
            {
                // stats.outside++;
                continue; // we're outside the triangle
            }
            // stats.inside++;

            // clipping #3 to viewport/projection space
            if (x < 0 || x >= width || y < 0 || y >= height)
            {
                // stats.xyClipped++;
                continue; // we're outside the viewport
            }

            // z-buffer (depth) check
            // get the z value for this point using the barymetric coordinates:
            float z = v1.z * u + v2.z * v + v3.z * w;
            rasterizer->drawPoint(vec3{(float)x, (float)y, z}, rgba_color);
        }
    }
}

} // minity
