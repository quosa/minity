#include <catch2/catch.hpp>
#include "imageImporter.h"

TEST_CASE("can import jpg file")
{
    minity::image img{};
    bool res = img.load("test/materials/newell_teapot.jpg");
    REQUIRE(res);
    REQUIRE(img.width == 600);
    REQUIRE(img.height == 453);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}

// https://docs.fileformat.com/image/jpeg/
TEST_CASE("can import jpg file without an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(res);
    REQUIRE(img.width == 1024);
    REQUIRE(img.height == 1024);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}

// https://docs.fileformat.com/image/tga/
TEST_CASE("can import tga file without an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/wall_512_3_05.tga");
    REQUIRE(res);
    REQUIRE(img.width == 512);
    REQUIRE(img.height == 512);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}

TEST_CASE("can import another tga file without an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/sample_640×426.tga");
    REQUIRE(res);
    REQUIRE(img.width == 640);
    REQUIRE(img.height == 426);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}

// https://docs.fileformat.com/image/png/
TEST_CASE("can import png file with an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/shovel.png");
    REQUIRE(res);
    REQUIRE(img.width == 853);
    REQUIRE(img.height == 1280);
    REQUIRE(img.components == 4); // HAS ALPHA CHANNEL!
}

TEST_CASE("can import png file without an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/sample_640×426.png");
    REQUIRE(res);
    REQUIRE(img.width == 640);
    REQUIRE(img.height == 426);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}

// https://docs.fileformat.com/image/bmp/
TEST_CASE("can import bmp file without an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/sample_1280×853.bmp");
    REQUIRE(res);
    REQUIRE(img.width == 1280);
    REQUIRE(img.height == 853);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
}


// https://docs.fileformat.com/image/tiff/
TEST_CASE("can import tif file with an alpha channel")
{
    minity::image img{};
    bool res = img.load("test/materials/TilesMarbleChevronCreamGrey001_IDMAP_1K.tif");
    REQUIRE(!res); // stb_image does not support tiff
    // REQUIRE(res);
    // REQUIRE(img.width == 1024);
    // REQUIRE(img.height == 1024);
    // REQUIRE(img.components == 4); // HAS ALPHA CHANNEL!
}

TEST_CASE("can import 2 files to different objects")
{
    bool res{false};
    minity::image img1{};
    minity::image img2{};
    res = img1.load("test/materials/sample_1280×853.bmp");
    REQUIRE(res);
    res = img2.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(res);
    REQUIRE(img1.width == 1280);
    REQUIRE(img1.height == 853);
    REQUIRE(img1.components == 3); // NO ALPHA CHANNEL!
    REQUIRE(img2.width == 1024);
    REQUIRE(img2.height == 1024);
    REQUIRE(img2.components == 3); // NO ALPHA CHANNEL!
}

TEST_CASE("can get color at (u,v) coordinate")
{
    minity::image img{};
    bool res = img.load("test/materials/texture_uvgrid01.jpg");
    REQUIRE(res);
    REQUIRE(img.width == 1024);
    REQUIRE(img.height == 1024);
    REQUIRE(img.components == 3); // NO ALPHA CHANNEL!
    u_int32_t color00 = img.get(0.0f, 0.0f);
    REQUIRE(color00 == 0xfbfcf7ff);
    u_int32_t color11 = img.get(1.0f, 1.0f);
    REQUIRE(color11 == 0xffffffff);
}

TEST_CASE("check non-flipped texture (u,v) corners")
{
    minity::image img{};
    bool res = img.load("test/materials/test_image_10x10.png");
    REQUIRE(res);
    REQUIRE(img.width == 10);
    REQUIRE(img.height == 10);
    REQUIRE(img.components == 4); // HAS ALPHA CHANNEL!
    u_int32_t color00 = img.get(0.0f, 0.0f);
    REQUIRE(color00 == 0xff0000ff); // red corner
    u_int32_t color10 = img.get(1.0f, 0.0f);
    REQUIRE(color10 == 0x000000ff); // black corner aabbggrr
    u_int32_t color01 = img.get(0.0f, 1.0f);
    REQUIRE(color01 == 0x00ff00ff); // green corner
    u_int32_t color11 = img.get(1.0f, 1.0f);
    REQUIRE(color11 == 0x0000ffff); // blue corner
}

TEST_CASE("check flipped texture (u,v) corners")
{
    minity::image img{};
    bool res = img.load("test/materials/test_image_10x10.png", true); // flipped vertically
    REQUIRE(res);
    REQUIRE(img.width == 10);
    REQUIRE(img.height == 10);
    REQUIRE(img.components == 4); // HAS ALPHA CHANNEL!
    u_int32_t color00 = img.get(0.0f, 0.0f);
    REQUIRE(color00 == 0x00ff00ff); // green corner
    u_int32_t color10 = img.get(1.0f, 0.0f);
    REQUIRE(color10 == 0x0000ffff); // blue corner
    u_int32_t color01 = img.get(0.0f, 1.0f);
    REQUIRE(color01 == 0xff0000ff); // red corner
    u_int32_t color11 = img.get(1.0f, 1.0f);
    REQUIRE(color11 == 0x000000ff); // black corner
}

TEST_CASE("check blue texture many (u,v) points")
{
    minity::image img{};
    bool res = img.load("test/materials/test_image_blue_100x100.png");
    REQUIRE(res);
    REQUIRE(img.width == 100);
    REQUIRE(img.height == 100);
    REQUIRE(img.components == 4);
    for (float u : {0.0f, 0.5f, 1.0f})
    {
        for (float v : {0.0f, 0.5f, 1.0f})
        {
            u_int32_t color00 = img.get(u, v);
            REQUIRE(color00 == 0x0000ffff); // blue
        }
    }
}
