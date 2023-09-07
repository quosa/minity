#include <catch2/catch.hpp>

#define MATH_TYPES_ONLY
#include "simpleMath.h"
#define MINITY_COLOR_TYPES_ONLY
#include "color.h"
#include "engine/software/rasterizer.h"

const vec3 origin{0.0f, 0.0f, 0.0f};

void printFramebuffer(minity::color *fb, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            std::cout << " " << fb[x + y * width];
        }
        std::cout << std::endl;
    }
}

TEST_CASE("init and dimensions")
{
    minity::rasterizer rasterizer(4,3);
    REQUIRE(rasterizer.getViewportWidth() == 4);
    REQUIRE(rasterizer.getViewportHeight() == 3);
}

TEST_CASE("can draw point to 0_0")
{
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawPoint(origin, minity::yellow);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    REQUIRE(fb[0] == minity::yellow);
    REQUIRE(fb[1] == minity::black);
    auto db = rasterizer.getDepthbuffer();
    REQUIRE(db[0] == 0.0f);
    REQUIRE(db[1] > 0.0f); // +inf
}

TEST_CASE("can draw point to width_height")
{
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawPoint(vec3{2.0f, 2.0f, 0.0f}, minity::yellow);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    REQUIRE(fb[3*3 - 1] == minity::yellow);
    REQUIRE(fb[0] == minity::black);
    REQUIRE(fb[3*3 - 2] == minity::black);
    auto db = rasterizer.getDepthbuffer();
    REQUIRE(db[3*3 - 1] == 0.0f);
    REQUIRE(db[3*3 - 2] > 0.0f); // +inf
}

TEST_CASE("z-buffer check")
{
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawPoint(vec3{1.0f, 1.0f, 0.96f}, minity::blue);
    rasterizer.drawPoint(vec3{1.0f, 1.0f, 0.42f}, minity::yellow);
    auto fb = rasterizer.getFramebuffer();
    // minity::yellow and not blue because o NDC/viewport
    // the camera looks at positive Z+ axis
    // so 0.96 for blue is further away than 0.42 for minity::yellow
    REQUIRE(fb[1 + 1*3] == minity::yellow);
    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT( db[1 + 1*3], Catch::Matchers::WithinRel(0.42f, 0.0000001f) );
}

TEST_CASE("draw line across the screen up")
{
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawLine(vec3{0.0f, 0.0f, 0.0f}, vec3{2.0f, 2.0f, 2.0f}, minity::white);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    REQUIRE(fb[0 + 0*3] == minity::white);
    REQUIRE(fb[1 + 1*3] == minity::white);
    REQUIRE(fb[2 + 2*3] == minity::white);

    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT(db[0 + 0*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[1 + 1*3], Catch::Matchers::WithinRel(1.0f, 0.0000001f) );
    REQUIRE_THAT(db[2 + 2*3], Catch::Matchers::WithinRel(2.0f, 0.0000001f) );

    REQUIRE(db[0 + 2*3] > 1000000); // +inf
}

TEST_CASE("draw line across the screen down")
{
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawLine(vec3{0.0f, 2.0f, 0.0f}, vec3{2.0f, 0.0f, 2.0f}, minity::white);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    REQUIRE(fb[0 + 2*3] == minity::white);
    REQUIRE(fb[1 + 1*3] == minity::white);
    REQUIRE(fb[2 + 0*3] == minity::white);

    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT(db[0 + 2*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[1 + 1*3], Catch::Matchers::WithinRel(1.0f, 0.0000001f) );
    REQUIRE_THAT(db[2 + 0*3], Catch::Matchers::WithinRel(2.0f, 0.0000001f) );
}

TEST_CASE("draw triangle top-left")
{
    vec3 vertices[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
    };
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawTriangle(vertices, minity::green);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    printFramebuffer(fb, 3, 3);
    minity::color b = minity::black;
    minity::color g = minity::green;
    minity::color exp[9]{
        g, g, g,
        g, g, b,
        g, b, b
    };
    printFramebuffer(exp, 3, 3);
    for (int i = 0; i < 9; ++i)
        REQUIRE(fb[i] == exp[i]);

    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT(db[0 + 0*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[2 + 0*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[0 + 2*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
}

TEST_CASE("draw triangle bottom-right")
{
    vec3 vertices[3]{
        {2.0f, 2.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
    };
    minity::rasterizer rasterizer(3,3);
    rasterizer.drawTriangle(vertices, minity::green);
    auto fb = rasterizer.getFramebuffer();
    // for (int i = 0; i < 3 * 3; ++i)
    // {
    //     std::cout << " " << std::to_string(fb[i]) << std::endl;
    // }
    printFramebuffer(fb, 3, 3);
    minity::color b = minity::black;
    minity::color g = minity::green;
    minity::color exp[9]{
        b, b, g,
        b, g, g,
        g, g, g
    };
    printFramebuffer(exp, 3, 3);
    for (int i = 0; i < 9; ++i)
        REQUIRE(fb[i] == exp[i]);

    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT(db[0 + 2*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[2 + 0*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
    REQUIRE_THAT(db[2 + 2*3], Catch::Matchers::WithinRel(0.0f, 0.0000001f) );
}

TEST_CASE("draw line bug - drawing does not stop")
{
    minity::rasterizer rasterizer(800, 800);
    // (429.847992, 168.462036, -4.037752)->(428.791626, 165.612427, -4.037769)
    // continued into an eternal loop (reason was a missing cast from float to int)
    rasterizer.drawLine(vec3{429.847992f, 168.462036f, -4.037752f}, vec3{428.791626, 165.612427, -4.037769}, minity::white);
    auto fb = rasterizer.getFramebuffer();
    // printFramebuffer(fb, 800, 800);

    // find out which indices have been painted
    // for (int i = 0; i < 800*800; ++i)
    // {
    //     if (fb[i] != minity::black)
    //     {
    //         std::cout << i << std::endl;
    //     }
    // }

    REQUIRE(fb[429 + 168*800] == minity::white);
    REQUIRE(fb[427 + 165*800] == minity::white);

    auto db = rasterizer.getDepthbuffer();
    REQUIRE_THAT(db[429 + 168*800], Catch::Matchers::WithinRel(-4.0f, 0.0000001f) );
    REQUIRE_THAT(db[427 + 165*800], Catch::Matchers::WithinRel(-4.0f, 0.0000001f) );
}

// WARNING: nan nan nan
//  x, y: 277,362
// (277.000000, 362.000000, 0.000000) // point
// (275.483643, 368.956848, 0.967554) // vertices
// (277.948761, 359.778381, 0.967689)
// (277.619690, 361.007660, 0.967300)

// in this case, the triangle is so thin that due to rounding
// the barycentric coordinates are nan/inf because
// the triangle area is zero and the denominator becomes zero.
// We'll discard and stat these cases separately
TEST_CASE("barycentric coordinates bug - degenerate triangle")
{
    float u{0};
    float v{0};
    float w{0};
    vec3 vertices[3]{
        vec3{275.483643, 368.956848, 0.967554},
        vec3{277.948761, 359.778381, 0.967689},
        vec3{277.619690, 361.007660, 0.967300}};
    vec3 point{277.000000, 362.000000, 0.000000};
    minity::barycentricCoordinates bc{};
    bc.prepare(vertices);
    bc.barycentricCoordinatesAt(vertices, point, u, v, w);

    std::cout << "v0 " << bc.v0 << " v1 " << bc.v1 << std::endl;
    std::cout << "v0 x v1 " << v3CrossProduct(bc.v0, bc.v1) << std::endl;
    std::cout << "inv denom " << bc.invDenom << std::endl;
    std::cout << "d00 * d11 " << bc.d00 * bc.d11 << std::endl;
    std::cout << "d01 * d01 " << bc.d01 * bc.d01 << std::endl;
    std::cout << "d00 * d11 - d01 * d01 " << bc.d00 * bc.d11 - bc.d01 * bc.d01 << std::endl;
    std::cout << "d00 " << bc.d00 << " d01 " << bc.d01 << " d11 " << bc.d11 << " d20 " << bc.d20 << " d21 " << bc.d21  << std::endl;
    REQUIRE(!std::isfinite(u)); // denominator becomes zero
    REQUIRE(!std::isfinite(v)); // denominator becomes zero
    REQUIRE(!std::isfinite(w)); // denominator becomes zero
}
