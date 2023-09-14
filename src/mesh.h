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

void printMesh(mesh &mesh);

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

// adapted from https://github.com/caosdoar/spheres/blob/master/src/spheres.cpp
// and https://danielsieger.com/blog/2021/03/27/generating-spheres.html
// and https://dev.to/ndesmic/webgl-3d-engine-from-scratch-part-6-procedural-sphere-generation-29bf
std::shared_ptr<minity::mesh> getSphereMesh(size_t meridians, size_t parallels)
{
    auto _mesh = std::make_shared<minity::mesh>();

    assert(parallels >= 2 && meridians >= 3);

    // GENERATE VERTICES (position, normal and texture coordinates)
    // from north pole down to south pole
    // even poles are represented by N(meridians) vertices
    // for texturing to work at the poles
    for (size_t p = 0; p <= parallels; p++)
    {
        float const polar = M_PI * float(p) / float(parallels);
        float const sp = std::sin(polar);
        float const cp = std::cos(polar);

        for (size_t m = 0; m <= meridians; m++)
        {
            float const azimuth = 2.0 * M_PI * float(m) / float(meridians);
            float const sa = std::sin(azimuth);
            float const ca = std::cos(azimuth);
            float const x = sp * ca;
            float const y = cp;
            float const z = sp * sa;
            float const u = (float(meridians - m)) / float(meridians);
            float const v = float(p) / float(parallels);
            _mesh->vertexData.emplace_back(
                minity::vertexData{
                {x, y, z},
                {x, y, z}, // unit circle so already normalized
                {u, v}
            });
        }
    }

    // GENERATE FACES (indices)
    for (size_t p = 0; p < parallels; p++)
    {
        size_t topParallelStart = p * (meridians + 1);
        size_t bottomParallelStart = (p + 1) * (meridians + 1);
        for (size_t m = 0; m < meridians; m++)
        {
            const u_int32_t topLeft = topParallelStart + m;
            const u_int32_t topRight = topParallelStart + (m + 1);
            const u_int32_t bottonLeft = bottomParallelStart + m;
            const u_int32_t bottomRight = bottomParallelStart + (m + 1);

            if (p == 0) // top-triangles - get 2 vertices from the next parallel
            {
                _mesh->indexData.insert(_mesh->indexData.end(), { topLeft, bottonLeft, bottomRight,  });
            }
            if (p > 0 && p < (parallels - 1)) // middle triangles - 2 faces
            {
                _mesh->indexData.insert(_mesh->indexData.end(), { topLeft, bottonLeft, bottomRight,  });
                _mesh->indexData.insert(_mesh->indexData.end(), { topLeft, bottomRight, topRight });
            }
            else // bottom triangles - get 2 vertices from previous parallel
            {
                _mesh->indexData.insert(_mesh->indexData.end(), { topLeft, bottomRight, topRight });
            }
        }
    }

    return _mesh;
}

void printMesh(mesh &mesh)
{
    std::cout << "mesh: " << std::endl;
    std::cout << "  vertices: " << std::endl;
    for (auto &vertex : mesh.vertexData)
    {
        std::cout << "    " << vertex.position.str() << std::endl;
    }
    std::cout << "  indices: " << std::endl;
    std::cout << "    ";
    for (auto &index : mesh.indexData)
    {
        std::cout << index << " ";
    }
    std::cout << std::endl;
}

#endif // MESH_UTILS_IMPLEMENTATION

} // NS minity
