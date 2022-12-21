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

// for texture coordinates (u, v)
struct vec2
{
    float u = 0;
    float v = 0;
};

struct model
{
    int numFaces{0};
    bool hasNormals{false};
    bool hasTextureCoordinates{false};
    std::vector<vec3> vertices; // x, y, z (w=1.0)
    std::vector<vec3> normals; // x, y, z (w=1.0)
    std::vector<vec2> textureCoordinates; // u, v (w ignored)
    std::vector<std::vector<int>> faces; // [[v1_idx, v2_idx, v3_idx], [...
    // during loading this houses {vertix_idx, normal_idx, texture_idx}, ...
    // u_int32_t color = 0xffffffff;

    bool load(const std::string &path);
private:
    bool handleLine(const std::string &line);
    bool alignFaces();
};

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
// load the mesh from an obj file
// supports vertices, faces, normals
// and texture coordinates
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
    if(!alignFaces())
    {
        std::cerr << "Trouble aligning faces" << std::endl;
        return false;
    }

    return true;
}

// utility to align vertices, normals
// and texture coordinates for easy access
bool model::alignFaces()
{
    // (deep-)copy the unsorted arrays
    std::vector<vec3> _vertices = vertices;
    std::vector<vec3> _normals = normals;
    std::vector<vec2> _textureCoordinates = textureCoordinates;
    std::vector<std::vector<int>> _faces = faces; // v1_idx, v2_idx, v3_idx, ...

    // reset the model arrays
    vertices.clear();
    normals.clear();
    textureCoordinates.clear();
    faces.clear();

    // populate the arrays back in order
    // so that each have same id for same vertex
    int i = 0;
    for (auto vnt : _faces)
    {
        vertices.insert( vertices.end(),
            { _vertices[vnt[0]], _vertices[vnt[3]], _vertices[vnt[6]]}
        );

        if (hasNormals)
        {
            normals.insert( normals.end(),
                {_normals[vnt[1]], _normals[vnt[4]], _normals[vnt[7]]}
            );
        }

        if (hasTextureCoordinates)
        {
            textureCoordinates.insert( textureCoordinates.end(),
                { _textureCoordinates[vnt[2]], _textureCoordinates[vnt[5]], _textureCoordinates[vnt[8]]}
            );
        }
        std::vector<int> face{ i, i+1, i+2 };
        faces.insert( faces.end(), face);
        i += 3;
    }
    numFaces = faces.size();
    return true;
}

// utility to handle a obj file line
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
        // e.g. f 1/11/111 2/22/222 3/33/333 (there are other types!)
        std::string vertexDetails;
        int vrt[50]; // vertex indices
        int nrm[50]; // normal indices
        int tex[50]; // texture cordinate indices
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
                indices.push_back(std::stoi(vertexDetails.substr(last, next - last)));
                last = next + 1;
            }
            indices.push_back(std::stoi(vertexDetails.substr(last)));

            // std::cout << "[";
            // for (auto elem : indices) { std::cout << " " << elem; };
            // std::cout << " ]" << std::endl;

            auto n = std::count(vertexDetails.begin(), vertexDetails.end(), '/');
            switch (n)
            {
            case 0: // just vertex indices: f 1 2 3
                vrt[i] = indices.at(0);
                nrm[i] = 0; // ends up as -1
                tex[i] = 0; // ends up as -1
                break;
            case 1: // vertex and texture coordinates: f 1/7 2/8 3/9
                vrt[i] = indices.at(0);
                tex[i] = indices.at(1);
                nrm[i] = 0; // ends up as -1
                hasTextureCoordinates = true;
                break;
            case 2: // vertex, texture coordinates and normals: f 1/7/12 2/8/13 3/9/14
                vrt[i] = indices.at(0);
                tex[i] = indices.at(1);
                nrm[i] = indices.at(2);
                hasNormals = true;
                hasTextureCoordinates = true;
                break;
            default:
                std::cerr << "Trouble reading face vertex information: " << vertexDetails << std::endl;
                assert(false); // we don't support anything else...
            }
            i++;
        }
        if (i>3)
        {
            std::cerr << "Warning, clipping a polygon of " << i << " vertices !!!" << std::endl;
        }

        // TODO: for 4-vertex polygon, do the triangulation manually?

        // pack the scattered indices to each face in v1, n1, t1, v2, n2, t2...
        std::vector<int> face;
        for (int i : {0, 1, 2})
        {
            face.insert( face.end(), { vrt[i] - 1, nrm[i] - 1, tex[i] -1} );
        }
        faces.push_back(face);

    }
    return true;
}

} // minity
