#pragma once

#include <cmath>
#include <limits>
#include <functional>

#define MINITY_SCENE_TYPES_ONLY
#include "../../simpleMath.h"
#include "../../color.h"
#include "stats.h"

namespace minity
{

const bool debugRasterizer{false};

typedef std::function< minity::color(float&, float&, float&, minity::color) > lambdaShader;
auto nullShader = [](float &u, float &v, float &w, minity::color color) -> minity::color { (void)u; (void)v; (void)w; return color; };

class rasterizer
{
public:
    void drawTriangle(const vec3 (&vertices)[3], const color rgba_color, lambdaShader fragmentShader=nullShader);
    void drawLine(const vec3 &from, const vec3 &to, const color rgba_color);
    void drawPoint(const vec3 &point, const color rgba_color);

    rasterizer(unsigned int width, unsigned int height)
        : viewportWidth(width), viewportHeight(height)
    {
        // both are addressed [x + y*viewportWidth]
        frameBuffer = std::vector<color>(viewportWidth * viewportHeight, minity::black);
        depthBuffer = std::vector<float>(viewportWidth * viewportHeight, std::numeric_limits<float>::infinity());
    };

    color *getFramebuffer();
    float *getDepthbuffer();
    unsigned int getViewportWidth();
    unsigned int getViewportHeight();
    void clearBuffers();

    rasterizerStats stats{0};

private:
    unsigned int viewportWidth{0};
    unsigned int viewportHeight{0};
    std::vector<color> frameBuffer;
    std::vector<float> depthBuffer;
    rasterizer(); // hide default constructor so that we get width and height
};

// fwd decl
void plotLine(const vec3 &from, const vec3 &to, const color rgba_color, rasterizer *rasterizer);
void plotTriangle(const vec3 (&vertices)[3], const color rgba_color, rasterizer *rasterizer, lambdaShader fragmentShader);

// method implementations

void rasterizer::clearBuffers()
{
    std::fill(frameBuffer.begin(), frameBuffer.end(), minity::black);
    std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<float>::infinity());
    stats = rasterizerStats{};
};
void rasterizer::drawTriangle(const vec3 (&vertices)[3], const color rgba_color, lambdaShader fragmentShader)
{
    if (debugRasterizer)
        std::cout << "drawTriangle: " << vertices[0] << " " << vertices[1] << " " << vertices[2] << std::endl;
    plotTriangle(vertices, rgba_color, this, fragmentShader);
    stats.triangles++;
};
void rasterizer::drawLine(const vec3 &from, const vec3 &to, const color rgba_color)
{
    // clipping #3 to viewport/projection space
    if (debugRasterizer)
        std::cout << "drawLine: " << from << "->" << to << std::endl;
    if (from.x < 0 || from.x >= viewportWidth || from.y < 0 || from.y >= viewportHeight
        || to.x < 0 || to.x >= viewportWidth || to.y < 0 || to.y >= viewportHeight)
    {
        stats.xyClipped++;
        return; // we're outside the viewport
    }

    plotLine(from, to, rgba_color, this);
    stats.lines++;
};
void rasterizer::drawPoint(const vec3 &point, color rgba_color)
{
    // assert(0 <= point.x && point.x <= viewportWidth);
    // assert(0 <= point.y && point.y <= viewportHeight);

    int x = (int)point.x;
    int y = (int)point.y;

    if (debugRasterizer)
        std::cout << "drawPoint: " << point << " (" << x << "," << y << ") ";

    // clipping #3 to viewport/projection space
    if (x < 0 || x >= (int)viewportWidth || y < 0 || y >= (int)viewportHeight)
    {
        stats.xyClipped++;
        if (debugRasterizer)
            std::cout << "clipped" << std::endl;
        return; // we're outside the viewport
    }

    // z-check
    if (point.z <= depthBuffer[x + y * viewportWidth])
    {
        frameBuffer[x + y * viewportWidth] = rgba_color;
        depthBuffer[x + y * viewportWidth] = point.z;
        stats.points++;
        if (debugRasterizer)
            std::cout << "drawn" << std::endl;
    }
    else
    {
        stats.depth++;
        if (debugRasterizer)
            std::cout << "z-clipped" << std::endl;
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
    // TODO: this is not good, it squashes the Z-buffer quite a lot
    // we can lose precision and depth order can be reversed
    // for z1=1.2 and z2=1.4, the deeper z2 will still be drawn :-(
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

// Barycentric:
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html (2d)

/* THIS IS JUST FOR REFERENCE:
void Barycentric(Point p, Point a, Point b, Point c, float &u, float &v, float &w)
{
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}
*/
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

// leave the components public as
// there's no real reason to hide them
// private:
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
void plotTriangle(const vec3 (&vertices)[3], const color rgba_color, rasterizer *rasterizer, lambdaShader fragmentShader)
{
    const vec3 &v1 = vertices[0];
    const vec3 &v2 = vertices[1];
    const vec3 &v3 = vertices[2];

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

            // likely a degenerate triangle (due to rounding the denominator becomes zero)
            if (!std::isfinite(u) || !std::isfinite(v) || !std::isfinite(w))
            {
                // std::cerr << "WARNING: non-finite barycentric coordinates!!!" << std::endl;
                // std::cerr << "WARNING: " << u << " " << v << " " << w << std::endl;
                rasterizer->stats.degenerate++;
                continue;
            }

            if (u < 0 || v < 0 || w < 0)
            {
                rasterizer->stats.outside++;
                continue; // we're outside the triangle
            }
            rasterizer->stats.inside++;

            // clipping #3 to viewport/projection space
            if (x < 0 || x >= width || y < 0 || y >= height)
            {
                rasterizer->stats.xyClipped++;
                continue; // we're outside the viewport
            }

            // z-buffer (depth) check
            // get the z value for this point using the barymetric coordinates:
            float z = v1.z * u + v2.z * v + v3.z * w;

            minity::color adjustedColor = fragmentShader(u, v, w, rgba_color);

            rasterizer->drawPoint(vec3{(float)x, (float)y, z}, adjustedColor);
        }
    }
}

} // NS minity
