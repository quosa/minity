#include <catch2/catch.hpp>

 // full scene implementation in one file,
 // other tests #define MINITY_SCENE_TYPES_ONLY
// #include "freezer/old_scene.h"
#include "scene.h"
#define IMAGEIMPORTER_IMPLEMENTATION
#include "imageImporter.h"

TEST_CASE("can import jpg file")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/newell_teapot.jpg");
    REQUIRE(img->width == 600);
    REQUIRE(img->height == 453);
    REQUIRE(img->components == 4); // forced rgba loading
}

// https://docs.fileformat.com/image/jpeg/
TEST_CASE("can import jpg file without an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(img->width == 1024);
    REQUIRE(img->height == 1024);
    REQUIRE(img->components == 4); // forced rgba loading
}

// https://docs.fileformat.com/image/tga/
TEST_CASE("can import tga file without an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/wall_512_3_05.tga");
    REQUIRE(img->width == 512);
    REQUIRE(img->height == 512);
    REQUIRE(img->components == 4); // forced rgba loading
}

TEST_CASE("can import another tga file without an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/sample_640×426.tga");
    REQUIRE(img->width == 640);
    REQUIRE(img->height == 426);
    REQUIRE(img->components == 4); // forced rgba loading
}

// https://docs.fileformat.com/image/png/
TEST_CASE("can import png file with an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/shovel.png");
    REQUIRE(img->width == 853);
    REQUIRE(img->height == 1280);
    REQUIRE(img->components == 4); // HAS ALPHA CHANNEL!
}

TEST_CASE("can import png file without an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/sample_640×426.png");
    REQUIRE(img->width == 640);
    REQUIRE(img->height == 426);
    REQUIRE(img->components == 4); // forced rgba loading
}

// https://docs.fileformat.com/image/bmp/
TEST_CASE("can import bmp file without an alpha channel")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/sample_1280×853.bmp");
    REQUIRE(img->width == 1280);
    REQUIRE(img->height == 853);
    REQUIRE(img->components == 4); // forced rgba loading
}


// https://docs.fileformat.com/image/tiff/
TEST_CASE("can import tif file with an alpha channel")
{
    minity::imageImporter importer{};
    std::string path = "test/materials/TilesMarbleChevronCreamGrey001_IDMAP_1K.tif";
    REQUIRE_THROWS( importer.load(path) ); // stb_image doesn't support tiff
}

TEST_CASE("can import 2 files to different objects")
{
    minity::imageImporter importer{};
    auto img1 = importer.load("test/materials/sample_1280×853.bmp");
    auto img2 = importer.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(img1->width == 1280);
    REQUIRE(img1->height == 853);
    REQUIRE(img1->components == 4); // forced rgba loading
    REQUIRE(img2->width == 1024);
    REQUIRE(img2->height == 1024);
    REQUIRE(img2->components == 4); // forced rgba loading
}

TEST_CASE("can get color at (u,v) coordinate")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(img->width == 1024);
    REQUIRE(img->height == 1024);
    REQUIRE(img->components == 4); // forced rgba loading
    u_int32_t color00 = img->get(0.0f, 0.0f);
    REQUIRE(color00 == 0xfbfcf7ff);
    u_int32_t color11 = img->get(1.0f, 1.0f);
    REQUIRE(color11 == 0xffffffff);
}

TEST_CASE("check non-flipped texture (u,v) corners")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/test_image_10x10.png");
    REQUIRE(img->width == 10);
    REQUIRE(img->height == 10);
    REQUIRE(img->components == 4); // HAS ALPHA CHANNEL!
    u_int32_t color00 = img->get(0.0f, 0.0f);
    REQUIRE(color00 == 0xff0000ff); // red corner
    u_int32_t color10 = img->get(1.0f, 0.0f);
    REQUIRE(color10 == 0x000000ff); // black corner aabbggrr
    u_int32_t color01 = img->get(0.0f, 1.0f);
    REQUIRE(color01 == 0x00ff00ff); // green corner
    u_int32_t color11 = img->get(1.0f, 1.0f);
    REQUIRE(color11 == 0x0000ffff); // blue corner
}

TEST_CASE("check flipped texture (u,v) corners")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/test_image_10x10.png", true); // flipped vertically
    REQUIRE(img->width == 10);
    REQUIRE(img->height == 10);
    REQUIRE(img->components == 4); // HAS ALPHA CHANNEL!
    u_int32_t color00 = img->get(0.0f, 0.0f);
    REQUIRE(color00 == 0x00ff00ff); // green corner
    u_int32_t color10 = img->get(1.0f, 0.0f);
    REQUIRE(color10 == 0x0000ffff); // blue corner
    u_int32_t color01 = img->get(0.0f, 1.0f);
    REQUIRE(color01 == 0xff0000ff); // red corner
    u_int32_t color11 = img->get(1.0f, 1.0f);
    REQUIRE(color11 == 0x000000ff); // black corner
}

TEST_CASE("check blue texture many (u,v) points")
{
    minity::imageImporter importer{};
    auto img = importer.load("test/materials/test_image_blue_100x100.png");
    REQUIRE(img->width == 100);
    REQUIRE(img->height == 100);
    REQUIRE(img->components == 4);
    for (float u : {0.0f, 0.5f, 1.0f})
    {
        for (float v : {0.0f, 0.5f, 1.0f})
        {
            u_int32_t color00 = img->get(u, v);
            REQUIRE(color00 == 0x0000ffff); // blue
        }
    }
}

// Assertion failed: (0 <= y && y < height), function get, file imageImporter.h, line 49.
TEST_CASE("check uv static_cast for 1x1 texture")
{
    struct test_scenario {float u, v; int x,y; };
    std::vector<test_scenario> tests{
        {0.0f, 0.0f, 0, 0},
        {1.0f, 0.0f, 0, 0},
        {0.0f, 1.0f, 0, 0},
        {1.0f, 1.0f, 0, 0}};
    int width = 1;
    int height = 1;
    for (auto t : tests)
    {
        int x = std::min(static_cast<int>(t.u * width), width - 1); // [0,width-1]
        int y = std::min(static_cast<int>(t.v * height), height-1); // [0, height-1]
        REQUIRE(x == t.x);
        REQUIRE(y == t.y);
    }
}

TEST_CASE("check uv static_cast for 2x2 texture")
{
    struct test_scenario {float u, v; int x, y; };
    std::vector<test_scenario> tests{
        {0.0f, 0.0f, 0, 0},
        {0.49f, 0.0f, 0, 0},
        {0.49999f, 0.0f, 0, 0},
        {0.50f, 0.0f, 1, 0}, // 0.5 * 2 = 1
        {0.99999f, 0.0f, 1, 0},
        {1.0f, 0.0f, 1, 0}, // 1.0 * 2 = 2 (min takes effect)
        {1.0f, 0.0f, 1, 0},
        {0.0f, 1.0f, 0, 1},
        {1.0f, 1.0f, 1, 1}};
    int width = 2;
    int height = 2;
    for (auto t : tests)
    {
        int x = std::min(static_cast<int>(t.u * width), width - 1); // [0,width-1]
        int y = std::min(static_cast<int>(t.v * height), height-1); // [0, height-1]
        REQUIRE(x == t.x);
        REQUIRE(y == t.y);
    }
}
