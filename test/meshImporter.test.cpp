#include <catch2/catch.hpp>

#define MATH_TYPES_ONLY
#include "simpleMath.h"
#include "mesh.h"
#define MESH_UTILS_IMPLEMENTATION
#include "meshImporter.h"

auto f3compare = [](auto &x, auto &y) { return x.x==y.x && x.y==y.y && x.z==y.z; };
auto f2compare = [](auto &x, auto &y) { return x.u==y.u && x.v==y.v; };

TEST_CASE("meshimporter - can import the Utah classic teapot")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/teapot.obj");
    REQUIRE(mesh->vertexData.size() == 6320 * 3 ); // only 3644 vertices but we need 6320 * 3 vertices
    REQUIRE(mesh->indexData.size() == 6320 * 3 ); // egrep "^f .*" test/models/teapot.obj | wc -l
}

TEST_CASE("meshimporter - can handle full face line")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/MaleLow.obj");
    REQUIRE(mesh->vertexData.size() == 3752 );
    REQUIRE(mesh->indexData.size() == 1494 * 3 ); // has many quads...
}

TEST_CASE("meshimporter - can import bigger free asset")
{
    minity::meshImporter importer;
    auto mesh = importer.load("test/models/Model_D0606058/head.obj");
    REQUIRE(mesh->vertexData.size() == 62864 );
    REQUIRE(mesh->indexData.size() == 31432 * 3 ); // has many quads...
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
    REQUIRE(mesh->vertexData.size() == 3);
    REQUIRE(mesh->indexData.size() == 3);
    vec3 vIexpected[3]{
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
    REQUIRE(mesh->vertexData.size() == 3);
    REQUIRE(mesh->indexData.size() == 3);
    vec3 vIexpected[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    vec3 nIexpected[3]{
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
    REQUIRE(mesh->vertexData.size() == 3);
    REQUIRE(mesh->indexData.size() == 3);
    vec3 vIexpected[3]{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };
    vec3 nIexpected[3]{
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}
    };
    vec2 tIexpected[3]{
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
