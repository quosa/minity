#include <catch2/catch.hpp>

// full implementation comes from imageImporter tests
#define MINITY_SCENE_TYPES_ONLY
#include "freezer/old_scene.h"
#include "freezer/modelImporter.h"

TEST_CASE("can import from memory")
{
    minity::modelImporter importer;
    auto mdl = importer.loadFromString("v 1.0 2.0 3.0\n");
    REQUIRE(mdl);
    REQUIRE(mdl->numFaces == 0);
    REQUIRE(!mdl->hasNormals);
    REQUIRE(!mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 0); // no faces so no alignment...
    REQUIRE(mdl->normals.size() == 0);
    REQUIRE(mdl->textureCoordinates.size() == 0);
    REQUIRE(mdl->faces.size() == 0);
}

TEST_CASE("can import the Utah classic teapot")
{
    minity::modelImporter importer;
    auto mdl = importer.load("test/models/teapot.obj");
    REQUIRE(mdl->numFaces == 6320);
    REQUIRE(!mdl->hasNormals);
    REQUIRE(!mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 18960); // was 3644;
    REQUIRE(mdl->normals.size() == 0); // no normals in this primitive model
    REQUIRE(mdl->textureCoordinates.size() == 0); // no normals in this primitive model
    REQUIRE(mdl->faces.size() == 6320);
}

TEST_CASE("can handle full face line")
{
    minity::modelImporter importer;
    auto mdl = importer.load("test/models/MaleLow.obj");
    REQUIRE(mdl->numFaces == 1496);
    REQUIRE(mdl->hasNormals);
    REQUIRE(mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 4488);
    REQUIRE(mdl->normals.size() == 4488);
    REQUIRE(mdl->textureCoordinates.size() == 4488);
    REQUIRE(mdl->faces.size() == 1496);
}

TEST_CASE("can import bigger free asset")
{
    minity::modelImporter importer;
    auto mdl = importer.load("test/models/Model_D0606058/head.obj");
    REQUIRE(mdl->numFaces == 31432);
    REQUIRE(mdl->hasNormals);
    REQUIRE(mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 94296);
    REQUIRE(mdl->normals.size() == 94296);
    REQUIRE(mdl->textureCoordinates.size() == 94296);
    REQUIRE(mdl->faces.size() == 31432);
}

TEST_CASE("detailed hand-written model")
{
    std::string modelString = \
    "v 0 0 0\n" // center
    "v 0 1 0\n" // up
    "v 1 0 0\n" // right
    "f 1 2 3\n"; // clockwise winding order
    minity::modelImporter importer;
    auto mdl = importer.loadFromString(modelString);
    REQUIRE(mdl->numFaces == 1);
    REQUIRE(!mdl->hasNormals);
    REQUIRE(!mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 3);
    REQUIRE(mdl->normals.size() == 0);
    REQUIRE(mdl->textureCoordinates.size() == 0);
    REQUIRE(mdl->faces.size() == 1);
}

TEST_CASE("detailed hand-written model with normals")
{
    std::string modelString = \
    "v 0 0 0\n" // center
    "v 0 1 0\n" // up
    "v 1 0 0\n" // right
    "vn 0 0 1\n" // center
    "vn 0 0 1\n" // up
    "vn 0 0 1\n" // right
    "f 1//1 2//2 3//3\n"; // clockwise winding order
    minity::modelImporter importer;
    auto mdl = importer.loadFromString(modelString);
    REQUIRE(mdl->numFaces == 1);
    REQUIRE(mdl->hasNormals);
    REQUIRE(!mdl->hasTextureCoordinates);
    REQUIRE(mdl->vertices.size() == 3);
    REQUIRE(mdl->normals.size() == 3);
    REQUIRE(mdl->textureCoordinates.size() == 0);
    REQUIRE(mdl->faces.size() == 1);

    // check the first face and normal
    REQUIRE(mdl->vertices[0] == vec3{0.0f, 0.0f, 0.0f});
    REQUIRE(mdl->normals[0] == vec3{0.0f, 0.0f, 1.0f});
}
