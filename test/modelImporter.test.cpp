#include <catch2/catch.hpp>
#include "modelImporter.h"

TEST_CASE("can import the Utah classic teapot")
{
    minity::model mdl{};
    bool res = mdl.load("test/models/teapot.obj");
    REQUIRE(res);
    REQUIRE(mdl.numFaces == 6320);
    REQUIRE(!mdl.hasNormals);
    REQUIRE(!mdl.hasTextureCoordinates);
    REQUIRE(mdl.vertices.size() == 18960); // was 3644;
    REQUIRE(mdl.normals.size() == 0); // no normals in this primitive model
    REQUIRE(mdl.textureCoordinates.size() == 0); // no normals in this primitive model
    REQUIRE(mdl.faces.size() == 6320);
}

TEST_CASE("can handle full face line")
{
    minity::model mdl{};
    bool res = mdl.load("test/models/MaleLow.obj");
    REQUIRE(res);
    REQUIRE(mdl.numFaces == 1126);
    REQUIRE(mdl.hasNormals);
    REQUIRE(mdl.hasTextureCoordinates);
    REQUIRE(mdl.vertices.size() == 3378); // was 810
    REQUIRE(mdl.normals.size() == 3378); // was 3752
    REQUIRE(mdl.textureCoordinates.size() == 3378); // was 1230
    REQUIRE(mdl.faces.size() == 1126);
}
