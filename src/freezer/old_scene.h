#pragma once

#define MATH_TYPES_ONLY
#include "../simpleMath.h"
#include "../scene.h" // new types

#include <memory>
#include <iomanip>
#include <cassert>

namespace minity
{


struct old_model
{
    // general fields
    bool enabled = true;
    std::string name{};

    // transformation fields
    vec3 scale{1.0f, 1.0f, 1.0f};
    vec3 rotation{};
    vec3 translation{};

    // model details
    int numFaces{0};
    bool hasNormals{false};
    bool hasTextureCoordinates{false};
    bool hasTexture{false};

    std::vector<std::vector<int>> faces; // [[v1_idx, v2_idx, v3_idx], [...
    std::vector<vec3> vertices; // x, y, z (w=1.0)
    std::vector<vec3> normals; // x, y, z (w=1.0)
    std::vector<vec2> textureCoordinates; // u, v (w ignored)
    std::shared_ptr<image> texture {nullptr};

    void printModelInfo(bool debug=false);
    void dumpModel();
    bool addTexture(std::shared_ptr<image> pTextureImage);
};

/*
scene
 - objects/nodes/thingies
    - meshes
        - faces (vertex_index/texture_index/normal_index) !!! start from index 1 !!!
            * basic: f 1 2 3
            * full: f 1/29/52 2/24/52 3/52/27
            * no normals: f v1/vt1 v2/vt2 v3/vt3 ...
            * no texture coordinates: f v1//vn1 v2//vn2 v3//vn3 ...
            * NOTE: can have polygons that have more than 3 vertices
        - vertices (x, y, z, [w]) v-lines, w=1.0 by default
            - ??? vertex colors, no w ???
        - normals (x,y,z) !!! might not be normalized to unit vectors !!!
        - texture coordinates (u, [v, w])
        - ??? Parameter space vertices ???
        - ??? Line element ???
*/

struct old_scene
{
    std::string name{};

    old_model model;
    camera camera;
    light light;
};



#ifndef MINITY_SCENE_TYPES_ONLY

bool old_model::addTexture(std::shared_ptr<minity::image> pTextureImage)
{
    // if (texture != nullptr)
    // {
    //     texture = nullptr;
    // }
    texture = pTextureImage;
    hasTexture = true;
    return true;
}

void  old_model::printModelInfo(bool debug)
{
    std::cout << "loaded a model with " << numFaces << " faces, ";
    std::cout << (hasNormals ? "has " : "no ") << "normals and ";
    std::cout << (hasTextureCoordinates ? "has " : "no ") << "texture coordinates";
    std::cout << "." << std::endl;

    if(debug)
    {
        std::cout << "vertices: [";
        for (auto v : vertices) { std::cout << " " << v; };
        std::cout << " ]" << std::endl;

        std::cout << "normals: [";
        for (auto n : normals) { std::cout << " " << n; };
        std::cout << " ]" << std::endl;

        std::cout << "texture coordinates: [";
        for (auto tc : textureCoordinates)
        {
            std::cout << " (" << tc.u << ", " << tc.v << ")";
        };
        std::cout << " ]" << std::endl;
    }
}

void  old_model::dumpModel()
{
    for (auto face : faces)
    {
        // v0, v1, v2 | n0, n1, n2 | t0, t1, t2
        for (int i : {0,1,2})
        {
            std::cout << " " << std::setw(35) << std::setprecision(3) << vertices[face[i]];
        };
        std::cout << " | ";
        if (hasNormals)
        {
            for (int i : {0,1,2})
            {
                std::cout << " " << std::setw(35) << std::setprecision(3) << normals[face[i]];
            };
            std::cout << " | ";
        }
        if (hasTextureCoordinates)
        {
            for (int i : {0,1,2})
            {
                auto tc = textureCoordinates[face[i]];
                // << std::setw(7) << std::setprecision(3)
                std::cout << " (" << tc.u << ", " << tc.v << ")" ;
            };

        }
        std::cout << std::endl;
    }
}

#endif //  MINITY_SCENE_TYPES_ONLY
} // minity
