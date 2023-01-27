#pragma once

#include "old_scene.h"

#include <cmath>
#include <cassert>
#include <vector>


namespace minity
{
// todo: sort out include order (vec3, tri and mesh from simpleMath.h)

// adapted from https://github.com/caosdoar/spheres/blob/master/src/spheres.cpp
void sphere(size_t meridians, size_t parallels, u_int32_t color, mesh &mesh)
{
    std::vector<vec3> vertices;

    // top / north pole
    vertices.emplace_back(vec3{0.0f, 1.0f, 0.0f});

    for (size_t j = 0; j < parallels - 1; ++j)
    {
        float const polar = M_PI * float(j + 1) / float(parallels);
        float const sp = std::sin(polar);
        float const cp = std::cos(polar);
        for (size_t i = 0; i < meridians; ++i)
        {
            float const azimuth = 2.0 * M_PI * float(i) / float(meridians);
            float const sa = std::sin(azimuth);
            float const ca = std::cos(azimuth);
            float const x = sp * ca;
            float const y = cp;
            float const z = sp * sa;
            vertices.emplace_back(vec3{x, y, z});
        }
    }

    // bottom / south pole
    vertices.emplace_back(vec3{0.0f, -1.0f, 0.0f});

    // top triangles
    for (size_t i = 0; i < meridians; ++i)
    {
        size_t const a = i + 1;
        size_t const b = (i + 1) % meridians + 1;
        mesh.tris.push_back(tri{
            {vertices[0], vertices[a], vertices[b]},
            color});
    }

    for (size_t j = 0; j < parallels - 2; ++j)
    {
        size_t aStart = j * meridians + 1;
        size_t bStart = (j + 1) * meridians + 1;
        for (size_t i = 0; i < meridians; ++i)
        {
            const size_t a = aStart + i;
            const size_t a1 = aStart + (i + 1) % meridians;
            const size_t b = bStart + i;
            const size_t b1 = bStart + (i + 1) % meridians;
            // mesh.addQuad(a, a1, b1, b);
            mesh.tris.push_back(tri{
                {vertices[b], vertices[a1], vertices[a]}, color});
            mesh.tris.push_back(tri{
                {vertices[b], vertices[b1], vertices[a1]}, color});
        }
    }

    // bottom triangles
    for (size_t i = 0; i < meridians; ++i)
    {
        size_t const a = i + meridians * (parallels - 2) + 1;
        size_t const b = (i + 1) % meridians + meridians * (parallels - 2) + 1;
        //mesh.addTriangle(mesh.vertices.size() - 1, a, b);
        mesh.tris.push_back(tri{
            {vertices[vertices.size() - 1], vertices[b], vertices[a]}, color});
    }
}

model square()
{
    model mdl{};
    mdl.numFaces = 12;
    mdl.hasNormals = true;
    mdl.hasTextureCoordinates = true;

    // clockwise winding order, i.e. center > up > right (left-hand rule!!!)
    std::vector<vec3> vertices{
        {-0.5f, -0.5f, 0.5f},
        {-0.5f,  0.5f, 0.5f},
        { 0.5f,  0.5f, 0.5f},
        { 0.5f, -0.5f, 0.5f}, // end of front corners (z = 0.5)
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f}, // end of back corners (z = -0.5)
    }; // x, y, z (w=1.0)
    std::vector<std::vector<int>> faces{
        {0, 1, 2}, {0, 2, 3}, // front
        {7, 6, 5}, {7, 5, 4}, // back
        {4, 5, 1}, {4, 1, 0}, // left
        {3, 2, 6}, {3, 6, 7}, // right
        {1, 5, 6}, {1, 6, 2}, // top
        {4, 0, 3}, {4, 3, 7}, // bottom
    }; // [[v1_idx, v2_idx, v3_idx], [...

    std::vector<std::vector<int>> faceTextureCoordinates{
        {0, 1, 2}, {0, 2, 3}, // front
        {0, 1, 2}, {0, 2, 3}, // back
        {0, 1, 2}, {0, 2, 3}, // left
        {0, 1, 2}, {0, 2, 3}, // right
        {0, 1, 2}, {0, 2, 3}, // top
        {0, 1, 2}, {0, 2, 3}, // bottom
    };

    std::vector<vec3> faceNormals{
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, // f
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, // b
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, // l
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, // r
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, // t
        {0.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 1.0f} // b
    }; // x, y, z (w=1.0)

    std::vector<vec2> textureCoordinates{
        {0.0f, 0.0f}, // 0 = BL
        {0.0f, 1.0f}, // 1 = TL
        {1.0f, 1.0}, // 2 = TR
        {1.0f, 0.0f} // 3 = BR
    }; // u, v (w ignored)

    int i = 0;
    std::vector<int> face{0};
    assert (faces.size() == faceNormals.size());
    int nIdx = 0;
    for (auto f = faces.begin(), tc = faceTextureCoordinates.begin();
        f != faces.end() && tc != faceTextureCoordinates.end();
        ++f, ++tc)
    {
        mdl.vertices.insert( mdl.vertices.end(), {
            vertices[(*f)[0]],
            vertices[(*f)[1]],
            vertices[(*f)[2]]
        });
        face.clear();
        face = {
            i,
            (i+1),
            (i+2)
        };
        mdl.faces.push_back(face);
        auto n = faceNormals[nIdx];
        mdl.normals.insert( mdl.normals.end(), {n , n, n});
        mdl.textureCoordinates.insert( mdl.textureCoordinates.end(), {
            textureCoordinates[(*tc)[0]],
            textureCoordinates[(*tc)[1]],
            textureCoordinates[(*tc)[2]]
        });

        i += 3;
        ++nIdx;
    }

    return mdl;
}

} // minity
