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
