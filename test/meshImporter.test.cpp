#include <catch2/catch.hpp>

// full implementation comes from imageImporter tests
// #define MINITY_SCENE_TYPES_ONLY
// #include "scene.h"

#include "mesh.h"
#include "meshImporter.h"

// utils to print vertices...
std::string sf3(simd::float3 sf3) { return std::to_string(sf3[0]) + " " + std::to_string(sf3[1]) + " " + std::to_string(sf3[2]); };
std::string sf2(simd::float2 sf2) { return std::to_string(sf2[0]) + " " + std::to_string(sf2[1]); };

auto f3compare = [](auto &x, auto &y) { return x.x==y.x && x.y==y.y && x.z==y.z; };
auto f2compare = [](auto &x, auto &y) { return x.x==y.x && x.y==y.y; };

TEST_CASE("meshimporter - can import the Utah classic teapot")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/teapot.obj");
    REQUIRE(mesh->vertexDataSize == 6320 * 3 * sizeof(VertexData) ); // only 3644 vertices but we need 6320 * 3 vertices
    REQUIRE(mesh->indexDataSize == 6320 * 3 * sizeof(u_int32_t) ); // egrep "^f .*" test/models/teapot.obj | wc -l
}

TEST_CASE("meshimporter - can handle full face line")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/MaleLow.obj");
    REQUIRE(mesh->vertexDataSize == 3752 * sizeof(VertexData) );
    REQUIRE(mesh->indexDataSize == 1494 * 3 * sizeof(u_int32_t) ); // has many quads...
}

TEST_CASE("meshimporter - can import bigger free asset")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/Model_D0606058/head.obj");
    REQUIRE(mesh->vertexDataSize == 62864 * sizeof(VertexData) );
    REQUIRE(mesh->indexDataSize == 31432 * 3 * sizeof(u_int32_t) ); // has many quads...
    // minity::modelImporter importer;
    // auto mdl = importer.load("test/models/Model_D0606058/head.obj");
    // REQUIRE(mdl->numFaces == 31432);
    // REQUIRE(mdl->hasNormals);
    // REQUIRE(mdl->hasTextureCoordinates);
    // REQUIRE(mdl->vertices.size() == 94296);
    // REQUIRE(mdl->normals.size() == 94296);
    // REQUIRE(mdl->textureCoordinates.size() == 94296);
    // REQUIRE(mdl->faces.size() == 31432);
}

TEST_CASE("meshimporter - detailed hand-written model")
{
    std::string modelString = \
    "v 0 0 0\n" // center
    "v 0 1 0\n" // up
    "v 1 0 0\n" // right
    "f 1 2 3\n"; // clockwise winding order
    minity::meshImporter importer;
    auto mesh = importer.loadFromString(modelString);
    REQUIRE(mesh->vertexDataSize == sizeof(VertexData) * 3);
    REQUIRE(mesh->indexDataSize == 3 * sizeof(u_int32_t)); // 3*4=12B
    simd::float3 vIexpected[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    for (int i = 0; i < 3; i++)
    {
        auto vI = mesh->vertexData[i].position;
        REQUIRE( f3compare(vI, vIexpected[i]) );
    }
}

TEST_CASE("meshimporter - detailed hand-written model with normals")
{
    std::string modelString = \
    "v 0 0 0\n" // center
    "v 0 1 0\n" // up
    "v 1 0 0\n" // right
    "vn 0 0 1\n" // center
    "vn 0 0 1\n" // up
    "vn 0 0 1\n" // right
    "f 1//1 2//2 3//3\n"; // clockwise winding order
    minity::meshImporter importer;
    auto mesh = importer.loadFromString(modelString);
    REQUIRE(mesh->vertexDataSize == sizeof(VertexData) * 3);
    REQUIRE(mesh->indexDataSize == 3 * sizeof(u_int32_t)); // 3*4=12B
    simd::float3 vIexpected[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    simd::float3 nIexpected[3]{
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    };
    for (int i = 0; i < 3; i++)
    {
        auto vI = mesh->vertexData[i].position;
        REQUIRE( f3compare(vI, vIexpected[i]) );
        auto nI = mesh->vertexData[i].normal;
        REQUIRE( f3compare(nI, nIexpected[i]) );
    }
}

TEST_CASE("meshimporter - detailed hand-written model with normals and texture coordinates")
{
    std::string modelString = \
    "v 0 0 0\n" // center
    "v 0 1 0\n" // up
    "v 1 0 0\n" // right
    "vn 0 0 1\n" // center
    "vn 0 0 1\n" // up
    "vn 0 0 1\n" // right
    "vt 0 1\n" // center
    "vt 0 0\n" // up
    "vt 1 0\n" // right
    "f 1/1/1 2/2/2 3/3/3\n"; // clockwise winding order
    minity::meshImporter importer;
    auto mesh = importer.loadFromString(modelString);
    REQUIRE(mesh->vertexDataSize == sizeof(VertexData) * 3);
    REQUIRE(mesh->indexDataSize == 3 * sizeof(u_int32_t)); // 3*4=12B
    simd::float3 vIexpected[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    simd::float3 nIexpected[3]{
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    };
    simd::float2 tIexpected[3]{
        {0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };
    for (int i = 0; i < 3; i++)
    {
        auto vI = mesh->vertexData[i].position;
        REQUIRE( f3compare(vI, vIexpected[i]) );
        auto nI = mesh->vertexData[i].normal;
        REQUIRE( f3compare(nI, nIexpected[i]) );
        auto tI = mesh->vertexData[i].texcoord;
        REQUIRE( f2compare(tI, tIexpected[i]) );
    }
}
