#pragma once

#define MATH_TYPES_ONLY
#include "simpleMath.h" // mesh etc. for now
#include <algorithm>
#include <iostream>
#include <fstream>
#include <strstream>
#include <string>

namespace minity
{

struct vec2
{
    float u = 0;
    float v = 0;
};

struct model
{
    int numVertices{0};
    bool hasNormals{false};
    bool hasTectureCoordinates{false};
    std::vector<vec3> vertices; // x, y, z (w=1.0)
    std::vector<vec3> normals; // x, y, z (w=1.0)
    std::vector<vec2> textureCoordinates; // u, v (w ignored)
    std::vector<tri> tris;
    // u_int32_t color = 0xffffffff;

    bool load(const std::string &path);
private:
    bool handleLine(const std::string &line);
};

// struct tri
// {
//     vec3 vertices[3];
//     vec3 normals[3];
//     vec3 textureCoordinates[3];
//     u_int32_t color = 0xffffffff;
// };

// struct mesh
// {
//     bool enabled = true;
//     std::vector<tri> tris;
//     vec3 scale{1.0f, 1.0f, 1.0f};
//     vec3 rotation{};
//     vec3 translation{};
// };


/*
scene
 - objects/nodes/thingies
    - meshes
        - faces (vertex_index/texture_index/normal_index) !!! start from index 1 !!!
            * basic: f 1 2 3
            * full: f 1/29/52 2/24/52 3/52/27
            * no normals: f v1/vt1 v2/vt2 v3/vt3 ...
            * no texture coordinates: f v1//vn1 v2//vn2 v3//vn3 ...
            * NOTE: can have polygons by having more then 3 items
        - vertices (x, y, z, [w]) v-lines, w=1.0 by default
            - ??? vertex colors, no w ???
        - normals (x,y,z) !!! might not be normalized to unit vectors !!!
        - texture coordinates (u, [v, w])
        - ??? Parameter space vertices ???
        - ??? Line element ???
*/


// adapted from: https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_olcEngine3D_Part3.cpp
bool model::load(const std::string &path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        std::cerr << "Trouble opening file: '" << path << "'" << std::endl;
        return false;
    }
    while (!f.eof())
    {
        std::string line;
        std::getline(f, line);

        // std::cout << line << std::endl;

        if(!handleLine(line))
        {
            std::cerr << "Trouble reading line: '" << line << "'" << std::endl;
            return false;
        }
    }
    return true;
}

bool model::handleLine(const std::string &line)
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
        s >> junk >> v.x >> v.y >> v.z;
        normals.push_back(v);
    }
    else if (line.rfind("vt ", 0) == 0) // texture coordinates
    {
        // e.g. vt 0.500 1 [0]
        vec2 v;
        s >> junk >> v.u >> v.v;
        textureCoordinates.push_back(v);
    }
    else if (line.rfind("f ", 0) == 0) // face
    {
        // std::cout << line << std::endl;

        std::string vertexDetails;
        int f[50]; // indices for triangle face...
        int i = 0;

        s >> junk;

        while (s >> vertexDetails)
        {
            // std::cout << vertexDetails << std::endl;

            std::vector<int> indices;
            size_t last = 0;
            size_t next = 0;
            // handle all possible scenarios here: 1, 1/2, 1/2/3, 1//3
            while ((next = vertexDetails.find('/', last)) != std::string::npos)
            {
                indices.push_back(std::stoi(vertexDetails.substr(last, next - last)));
                last = next + 1;
            }
            indices.push_back(std::stoi(vertexDetails.substr(last)));

            // std::cout << "[";
            // for (auto elem : indices) { std::cout << elem << ", "; };
            // std::cout << "]" << std::endl;

            auto n = std::count(vertexDetails.begin(), vertexDetails.end(), '/');
            switch (n)
            {
            case 0: // just vertex indices: f 1 2 3
                f[i] = indices.at(0);
                break;
            case 1: // vertex and texture coordinates: f 1/7 2/8 3/9
                f[i] = indices.at(0);
                break;
            case 2: // vertex, texture coordinates and normals: f 1/7/12 2/8/13 3/9/14
                f[i] = indices.at(0);
                break;
            default:
                std::cerr << "Trouble reading face vertex information: " << vertexDetails << std::endl;
                assert(false); // we don't support anything else...
            }
            i++;
        }
        u_int32_t color = 0xffffffff;
        tris.push_back({{vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1]}, color});
    }
    return true;
}

} // minity
