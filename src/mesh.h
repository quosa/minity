#pragma once

namespace minity
{

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

#ifdef MESH_UTILS_IMPLEMENTATION

std::shared_ptr<minity::mesh> getSingleFaceMesh()
{
    auto _mesh = std::make_shared<minity::mesh>();
    _mesh->indexData = {0, 1, 2};
    _mesh->vertexData = {
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -1.0f, -1.0f, 0.0f }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { -1.0f, 1.0f, 0.0f },  {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
        { { 1.0f, 1.0f, 0.0f },   {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
    };
    return _mesh;
}

std::shared_ptr<minity::mesh> getSquareMesh()
{
    auto _mesh = std::make_shared<minity::mesh>();
    _mesh->indexData = {0, 1, 2, 0, 2, 3};
    _mesh->vertexData = {
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -1.0f, -1.0f, 0.0f }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { -1.0f, 1.0f, 0.0f },  {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
        { { 1.0f, 1.0f, 0.0f },   {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
        { { 1.0f, -1.0f, 0.0f },  {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
    };
    return _mesh;
}

std::shared_ptr<minity::mesh> getCubeMesh()
{
    const float s = 0.5f;

    auto _mesh = std::make_shared<minity::mesh>();
    _mesh->indexData = {
        0,  1,  2,  2,  3,  0, /* front */
        4,  5,  6,  6,  7,  4, /* right */
        8,  9, 10, 10, 11,  8, /* back */
        12, 13, 14, 14, 15, 12, /* left */
        16, 17, 18, 18, 19, 16, /* top */
        20, 21, 22, 22, 23, 20, /* bottom */
    };
    _mesh->vertexData = {
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -s, -s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { -s, +s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
        { { +s, +s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
        { { +s, -s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },

        { { +s, -s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { +s, +s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 0.f } },
        { { +s, +s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { +s, -s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 1.f } },

        { { +s, -s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },
        { { +s, +s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },
        { { -s, +s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
        { { -s, -s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },

        { { -s, -s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { -s, +s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 0.f } },
        { { -s, +s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { -s, -s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 1.f } },

        { { -s, +s, +s }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },
        { { -s, +s, -s }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
        { { +s, +s, -s }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
        { { +s, +s, +s }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },

        { { -s, -s, -s }, {  0.f, -1.f,  0.f }, { 0.f, 1.f } },
        { { -s, -s, +s }, {  0.f, -1.f,  0.f }, { 0.f, 0.f } },
        { { +s, -s, +s }, {  0.f, -1.f,  0.f }, { 1.f, 0.f } },
        { { +s, -s, -s }, {  0.f, -1.f,  0.f }, { 1.f, 1.f } },
    };
    return _mesh;
}

#endif // MESH_UTILS_IMPLEMENTATION

} // NS minity
