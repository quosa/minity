#pragma once

#define MATH_TYPES_ONLY
#include "simpleMath.h"
// #define MINITY_SCENE_TYPES_ONLY
// #include "old_scene.h"
#include "mesh.h"

#include <simd/simd.h> // vector_floatN

#include <algorithm>
#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace minity
{

struct meshImporter
{
public:
    meshImporter() : m_mesh(std::make_shared<mesh>()) {}
    ~meshImporter() { m_mesh.reset(); }
    std::shared_ptr<mesh> load(const std::string &path, bool reverseWinding=false);
    std::shared_ptr<mesh> loadFromString(const std::string &meshString, bool reverseWinding=false);
private:
    void handleLine(const std::string &line, bool reverseWinding);
    std::shared_ptr<mesh> m_mesh{nullptr};

    std::vector<vec3> vertices{};
    std::vector<vec3> normals{};
    std::vector<vec2> textureCoordinates{};
    std::vector<VertexData> metalVertices{};
    std::vector<u_int32_t> metalIndices{};
};

// adapted originally from:
// https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_olcEngine3D_Part3.cpp
// load the mesh from an obj file
// supports vertices, faces, normals
// and texture coordinates
// might throw runtime exception
std::shared_ptr<mesh> meshImporter::load(const std::string &path, bool reverseWinding)
{
    if (m_mesh)
    {
        m_mesh.reset();
        m_mesh = std::make_shared<mesh>();
    }
    std::ifstream f(path);
    if (!f.is_open())
    {
        // std::cerr << "Trouble opening file: '" << path << "'" << std::endl;
        throw std::runtime_error("Trouble opening file: '" + path + "'");
    }
    std::string line;
    while (!f.eof())
    {
        // std::cout << line << std::endl;
        std::getline(f, line);
        handleLine(line, reverseWinding);
    }
    // m_mesh->printModelInfo();

    std::cout << "loaded a model with " << metalVertices.size() << " vertices,";
    std::cout << " and " <<  metalIndices.size() / 3 << " faces." << std::endl;

    m_mesh->vertexDataSize = metalVertices.size() * sizeof(VertexData);
    m_mesh->vertexData = new VertexData[m_mesh->vertexDataSize];
    memcpy(m_mesh->vertexData, &metalVertices[0], m_mesh->vertexDataSize);

    m_mesh->indexDataSize = metalIndices.size() * sizeof(u_int32_t);
    m_mesh->indexData = new u_int32_t[m_mesh->indexDataSize];
    memcpy(m_mesh->indexData, &metalIndices[0], m_mesh->indexDataSize);

    return m_mesh;
}

// might throw runtime exception
std::shared_ptr<mesh> meshImporter::loadFromString(const std::string &meshString, bool reverseWinding)
{
    if (m_mesh)
    {
        m_mesh.reset();
        m_mesh = std::make_shared<mesh>();
    }
    std::istringstream iss(meshString);
    std::string line;
    while (std::getline(iss, line)) {
        // std::cout << line << std::endl;
        handleLine(line, reverseWinding);
    }
    // m_mesh->printModelInfo();

    m_mesh->vertexDataSize = metalVertices.size() * sizeof(VertexData);
    m_mesh->vertexData = new VertexData[m_mesh->vertexDataSize];
    memcpy(m_mesh->vertexData, &metalVertices[0], m_mesh->vertexDataSize);

    m_mesh->indexDataSize = metalIndices.size() * sizeof(u_int32_t);
    m_mesh->indexData = new u_int32_t[m_mesh->indexDataSize];
    memcpy(m_mesh->indexData, &metalIndices[0], m_mesh->indexDataSize);

    return m_mesh;
}

// utility to handle a obj file line
void meshImporter::handleLine(const std::string &line, bool reverseWinding)
{
    std::strstream s;
    s << line;

    char junk;

    // TODO: wait for c++20 starts_with
    if (line.rfind("v ", 0) == 0) // vertex
    {
        // e.g. v 0.123 0.234 0.345 1.0
        vec3 v;
        s >> junk >> v.x >> v.y >> v.z;
        vertices.push_back(v);
    }
    else if (line.rfind("vn ", 0) == 0) // normal
    {
        // e.g. vn 0.707 0.000 0.707
        vec3 v;
        s >> junk >> junk >> v.x >> v.y >> v.z;
        normals.push_back(v);
    }
    else if (line.rfind("vt ", 0) == 0) // texture coordinates
    {
        // e.g. vt 0.500 1 [0]
        vec2 v;
        s >> junk >> junk >> v.u >> v.v;
        if (v.u > 1.0f && v.u <= 2.0f)
            v.u -= 1.0f; // likely indicates tiling
        if (v.v > 1.0f && v.v <= 2.0f)
            v.v -= 1.0f; // likely indicates tiling
        if (v.u < 0.0f && v.u >= -1.0f)
            v.u += 1.0f; // likely indicates tiling
        if (v.v < 0.0f && v.v >= -1.0f)
            v.v += 1.0f; // likely indicates tiling
        textureCoordinates.push_back(v);
    }
    else if (line.rfind("f ", 0) == 0) // face
    {
        // e.g. f 1/11/111 2/22/222 3/33/333 (there are other types!)
        std::string vertexDetails;
        int i = 0;

        s >> junk;

        while (s >> vertexDetails)
        {
            std::vector<int> indices;
            size_t last = 0;
            size_t next = 0;
            // handle all possible scenarios here: 1, 1/2, 1/2/3, 1//3
            while ((next = vertexDetails.find('/', last)) != std::string::npos)
            {
                int idx = 0; // case f 1//3
                if ((next - last) > 0)
                    idx = std::stoi(vertexDetails.substr(last, next - last));
                indices.push_back(idx);
                last = next + 1;
            }
            indices.push_back(std::stoi(vertexDetails.substr(last)));

            // std::cout << "indices: [";
            // for (auto elem : indices) { std::cout << " " << elem; };
            // std::cout << " ]" << std::endl;

            vec3 vert;
            vec3 nrm;
            vec2 tex;
            vec3 zero3{}; // for filling in blanks
            vec2 zero2{}; // for filling in blanks
            auto n = std::count(vertexDetails.begin(), vertexDetails.end(), '/');
            switch (n)
            {
            case 0: // just vertex indices: f 1 2 3
                vert = indices.at(0) > 0 ? vertices[indices.at(0) - 1] : zero3;
                nrm = zero3;
                tex = zero2;
                break;
            case 1: // vertex and texture coordinates: f 1/7 2/8 3/9
                vert = indices.at(0) > 0 ? vertices[indices.at(0) - 1] : zero3;
                nrm = zero3;
                tex = indices.at(1) > 0 ? textureCoordinates[indices.at(1) - 1] : zero2;
                // m_mesh->hasTextureCoordinates = true;
                break;
            case 2: // vertex, texture coordinates and normals: f 1/7/12 2/8/13 3/9/14
                vert =indices.at(0) > 0 ? vertices[indices.at(0) - 1] : zero3;
                nrm = indices.at(2) > 0 ? normals[indices.at(2) - 1] : zero3;
                tex = indices.at(1) > 0 ? textureCoordinates[indices.at(1) - 1] : zero2;
                // m_mesh->hasNormals = true;
                // m_mesh->hasTextureCoordinates = (textureCoordinateIndex[i] != 0);
                break;
            default:
                // std::cerr << "Trouble reading face vertex information: " << vertexDetails << std::endl;
                throw std::runtime_error("Trouble reading face vertex information: " + vertexDetails); // we don't support anything else...
            }

            VertexData vertex{
                {vert.x, vert.y, vert.z},
                {nrm.x, nrm.y, nrm.z},
                {tex.u, tex.v}
            };

            metalVertices.push_back(vertex);
            i++;
        };

        int j = metalVertices.size();
        if (i == 3)
        {
            metalIndices.insert( metalIndices.end(), { (u_int32_t)(j - 3),  (u_int32_t)(j - 2),  (u_int32_t)(j - 1)});
            if (reverseWinding)
            {
                assert(metalIndices.size() > 2);
                std::swap(metalIndices.end()[-2], metalIndices.end()[-1]);
            }
        }
        else if (i == 4)
        {
            metalIndices.insert( metalIndices.end(), { (u_int32_t)(j - 4),  (u_int32_t)(j - 3),  (u_int32_t)(j - 2)});
            if (reverseWinding)
            {
                assert(metalIndices.size() > 2);
                std::swap(metalIndices.end()[-2], metalIndices.end()[-1]);
            }
            metalIndices.insert( metalIndices.end(), { (u_int32_t)(j - 4),  (u_int32_t)(j - 2),  (u_int32_t)(j - 1)});
            if (reverseWinding)
            {
                assert(metalIndices.size() > 2);
                std::swap(metalIndices.end()[-2], metalIndices.end()[-1]);
            }
        }
        else if (i>4)
        {
            std::cerr << "Warning, clipping a polygon of " << i << " vertices !!!" << std::endl;
        }
    }
}

} // minity
