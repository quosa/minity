#include <catch2/catch.hpp>

#define MATH_TYPES_ONLY
#include "simpleMath.h"

#include "engine/metal/shader_types.h"

struct vertexData
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

struct mesh
{
    std::vector<vertexData> vertexData;
    std::vector<u_int32_t> indexData;
};

struct metal_mesh
{
    VertexData *vertexData;
    size_t vertexDataSize;
    u_int32_t *indexData;
    size_t indexDataSize;
};

metal_mesh &convertMeshToMetal(mesh &mesh)
{
    auto mm = new metal_mesh();

    mm->vertexDataSize = mesh.vertexData.size() * sizeof(VertexData);
    mm->vertexData = new VertexData[mesh.vertexData.size()];

    mm->indexDataSize = mesh.indexData.size() * sizeof(u_int32_t);
    mm->indexData = new u_int32_t[mesh.indexData.size()];

    size_t i = 0;
    for(auto v : mesh.vertexData)
    {
        auto p = v.position;
        auto n = v.normal;
        auto t = v.texcoord;
        VertexData vd{{p.x, p.y, p.z}, {n.x, n.y, n.z}, {t.u, t.v}};
        mm->vertexData[i] = vd;
        i++;
    }

    size_t j = 0;
    for(auto idx : mesh.indexData)
    {
        mm->indexData[j] = idx;
        j++;
    }
    return *mm;
};

TEST_CASE("vertex - create and convert")
{
    vertexData vd{{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}};
    u_int32_t idx{0};
    mesh m{};
    m.vertexData.push_back(vd);
    m.indexData.push_back(idx);
    REQUIRE( m.vertexData.size() == 1);
    REQUIRE( m.indexData.size() == 1);

    auto mm = convertMeshToMetal(m);

    REQUIRE(mm.vertexDataSize == 1 * sizeof(VertexData));
    REQUIRE(mm.indexDataSize == 1 * sizeof(u_int32_t));

    simd::float3 expectedPosition{1.0f, 2.0f, 3.0f};
    simd::float3 expectedNormal{0.0f, 0.0f, 1.0f};
    simd::float2 expectedTexcoord{0.0f, 0.0f};
    REQUIRE(simd_equal(mm.vertexData[0].position, expectedPosition));
    REQUIRE(simd_equal(mm.vertexData[0].normal, expectedNormal));
    REQUIRE(simd_equal(mm.vertexData[0].texcoord, expectedTexcoord));

    REQUIRE(mm.indexData[0] == (u_int32_t)0);
}

TEST_CASE("vertex - create and convert face")
{
    vertexData vd0{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}};
    vertexData vd1{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}};
    vertexData vd2{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}};
    mesh m{};
    m.vertexData.insert( m.vertexData.end(), { vd0, vd1, vd2 });
    m.indexData.insert( m.indexData.end(), { 0, 1, 2 });
    REQUIRE( m.vertexData.size() == 3);
    REQUIRE( m.indexData.size() == 3);

    auto mm = convertMeshToMetal(m);

    REQUIRE(mm.vertexDataSize == 3 * sizeof(VertexData));
    REQUIRE(mm.indexDataSize == 3 * sizeof(u_int32_t));

    simd::float3 expectedPosition{0.0f, 0.0f, 0.0f};
    simd::float3 expectedNormal{0.0f, 0.0f, 1.0f};
    simd::float2 expectedTexcoord{1.0f, 1.0f};
    REQUIRE(simd_equal(mm.vertexData[0].position, expectedPosition));
    REQUIRE(simd_equal(mm.vertexData[1].normal, expectedNormal));
    REQUIRE(simd_equal(mm.vertexData[2].texcoord, expectedTexcoord));

    REQUIRE(mm.indexData[0] == (u_int32_t)0);
    REQUIRE(mm.indexData[1] == (u_int32_t)1);
    REQUIRE(mm.indexData[2] == (u_int32_t)2);
}

