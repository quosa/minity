#pragma once

#include "simpleMath.h" // mesh etc. for now
#include "imageImporter.h"

#include <iomanip>

namespace minity
{

struct camera
{
    float fovDegrees = 54.4; // 36x24mm 35mm film frame / full sensor
    vec3 rotation{0}; // no rotations
    vec3 translation{0.0f, 0.0f, 1.0f}; // on positive z-axis
    vec3 lookAt{0.0f, 0.0f, 0.0f}; // looking at origin
    vec3 up{0.0f, 1.0f, 0.0f}; // y-axis is up
    mat4 getCameraMatrix();
    mat4 getFPSCameraMatrix();
private:
    mat4 cameraMatrix;
};

struct light
{
    vec3 rotation{0};
    vec3 translation{0};
    mat4 getLightTranslationMatrix();
private:
    mat4 lightMatrix;
};

struct model
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
    std::shared_ptr<minity::image> texture {nullptr};

    void printModelInfo(bool debug=false);
    void dumpModel();
    bool addTexture(std::shared_ptr<minity::image> pTextureImage);
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

struct scene
{
    std::string name{};

    model model;
    camera camera;
    light light;
};




#ifndef MINITY_SCENE_TYPES_ONLY

mat4 camera::getCameraMatrix()
{
    cameraMatrix = lookAtMatrixRH(translation, lookAt, up);
    return cameraMatrix;
}

mat4 camera::getFPSCameraMatrix()
{
    // pitch -90 .. 90, yaw 0 ... 360, (both in rad)
    cameraMatrix = fpsLookAtMatrixRH(translation, rotation.x, rotation.y);
    return cameraMatrix;
}

mat4 light::getLightTranslationMatrix()
{
    // light transformations
    mat4 lightXRotator = rotateXMatrix(rotation.x);
    mat4 lightYRotator = rotateYMatrix(rotation.y);
    mat4 lightZRotator = rotateZMatrix(rotation.z);
    mat4 lightTranslator = translateMatrix(translation.x, translation.y, translation.z);

    // order matters: scale > rotate > move (=translate)
    // TODO: THINK IF WE NEED SCALE
    mat4 lightTransformations = multiplyMat4(lightYRotator, lightXRotator);
    lightTransformations = multiplyMat4(lightZRotator, lightTransformations);
    lightTransformations = multiplyMat4(lightTranslator, lightTransformations);
    lightMatrix = lightTransformations;
    return lightMatrix;
}

//    vec3 lightRay = v3Normalize(multiplyVec3(vec3{0.0f, -1.0f, 0.0f}, lightTransformations));
//    lightRay.z = 1; // todo: use the light entity for real


bool model::addTexture(std::shared_ptr<minity::image> pTextureImage)
{
    if (texture != nullptr)
    {
        texture = nullptr;
    }
    texture = pTextureImage;
    hasTexture = true;
    return true;
}

void  model::printModelInfo(bool debug)
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

void  model::dumpModel()
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
