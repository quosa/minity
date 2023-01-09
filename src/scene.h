#pragma once

#define MATH_TYPES_ONLY
#include "simpleMath.h" // mesh etc. for now

#include <memory>
#include <iomanip>
#include <cassert>

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

struct image
{
    int width{0};
    int height{0};
    int components{0}; // typically rgb = 3 or rgba = 4
    u_int32_t get(float u, float v) const;
    void set(unsigned char *_raw_data);
    ~image() { free(_raw_data); } // stbi_image_free is free by default
private:
    unsigned char *_raw_data{nullptr};
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
    // if (texture != nullptr)
    // {
    //     texture = nullptr;
    // }
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

// stb_image: the first pixel pointed to is top-left-most in the image
// 0,0 = top-left (or bottom-left if flipped vertically)
// 1,0 = top-right (or bottom-right if flipped vertically)
// 0,1 = bottom-left (or top-left if flipped vertically)
// 1,1 = bottom-right (or top-right if flipped vertically)
u_int32_t image::get(float u, float v) const
{
    int x = std::min(static_cast<int>(u * width), width - 1); // [0,width - 1]
    int y = std::min(static_cast<int>(v * height), height-1); // [0, height - 1]
    assert(0 <= x && x < width);
    assert(0 <= y && y < height);
    assert(_raw_data != nullptr);

    auto data = _raw_data + (x + y * width) * components;
    // stb_image: comp 3 are red, green, blue
    // stb_image: comp 4 are red, green, blue, alpha
    // this sets the output to 0xrrggbbaa
    u_int32_t color = (data[3]<<0) | (data[2]<<8) | (data[1]<<16) | ((unsigned)data[0]<<24);
    if (components == 3)
    {
        color = color | 0xff;
    }

    return color;
}

// set data buffer
void image::set(unsigned char *data)
{
    _raw_data = data;
}

#endif //  MINITY_SCENE_TYPES_ONLY
} // minity
