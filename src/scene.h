#pragma once

#define MATH_TYPES_ONLY
#include "simpleMath.h" // vec3, mat4
#include "mesh.h"
#include "color.h"

#include <memory>
#include <iomanip>
#include <cassert>

namespace minity
{

struct camera
{
    float fovDegrees = 54.4;            // 36x24mm 35mm film frame / full sensor
    vec3 rotation{0};                   // no rotations
    vec3 translation{0.0f, 0.0f, 1.0f}; // on positive z-axis
    vec3 lookAt{0.0f, 0.0f, 0.0f};      // looking at origin
    vec3 up{0.0f, 1.0f, 0.0f};          // y-axis is up
    mat4 getCameraMatrix();
    mat4 getFPSCameraMatrix();

private:
    mat4 cameraMatrix;
};

struct light
{
    vec3 rotation{0};
    vec3 translation{0};
    mat4 getLightTransformationMatrix();

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
    void *_getRawData();
    ~image() { free(_raw_data); } // stbi_image_free is free by default

private:
    unsigned char *_raw_data{nullptr};
};

typedef image texture; // TODO: change for real...

struct material
{
    color color; // rgba - 0xffff00ff = yellow
    float reflectivity;
    // transparency and refraction later
    texture &texture;
};

struct model; // fwd
typedef std::function<void(float)> updateMethod;
typedef std::function<updateMethod(minity::model *)> updateMethodFactory;
// default null-update method that does nothing
auto nullUpdater = [](float timeDelta) -> void { (void)timeDelta; };

struct model
{
    // TODO: consider re-using these generic fields
    // // general fields
    // bool enabled = true;
    // std::string name{};
    mesh mesh;
    material material;
    vec3 position{};
    vec3 rotation{};
    vec3 scale{};
    mat4 getModelTransformMatrix();
    updateMethod update = nullUpdater;
    void setUpdate(updateMethodFactory updateMaker) { update = updateMaker(this); }
};

struct scene
{
    // TODO: consider re-using these generic fields
    // std::string name{};
    // TODO: turn to vectors?
    camera &camera;
    light &light;
    model &model;
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

mat4 light::getLightTransformationMatrix()
{
    // light transformations
    mat4 lightXRotator = rotateXMatrix(rotation.x);
    mat4 lightYRotator = rotateYMatrix(rotation.y);
    mat4 lightZRotator = rotateZMatrix(rotation.z);
    mat4 lightTranslator = translateMatrix(translation.x, translation.y, translation.z);

    // order matters: scale > rotate > move (=translate)
    // TODO: think if we need a scale component in light?
    mat4 lightTransformations = multiplyMat4(lightYRotator, lightXRotator);
    lightTransformations = multiplyMat4(lightZRotator, lightTransformations);
    lightTransformations = multiplyMat4(lightTranslator, lightTransformations);
    lightMatrix = lightTransformations;
    return lightMatrix;
}

mat4 model::getModelTransformMatrix()
{
    mat4 scaler = scaleMatrix(scale.x, scale.y, scale.z);
    mat4 xRotator = rotateXMatrix(rotation.x);
    mat4 yRotator = rotateYMatrix(rotation.y);
    mat4 zRotator = rotateZMatrix(rotation.z);
    mat4 translator = translateMatrix(position.x, position.y, position.z);

    // order matters: scale > rotate > move (=translate)
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);
    return worldTransformations;
}

// stb_image: the first pixel pointed to is top-left-most in the image
// 0,0 = top-left (or bottom-left if flipped vertically)
// 1,0 = top-right (or bottom-right if flipped vertically)
// 0,1 = bottom-left (or top-left if flipped vertically)
// 1,1 = bottom-right (or top-right if flipped vertically)
u_int32_t image::get(float u, float v) const
{
    int x = std::min(static_cast<int>(u * width), width - 1);   // [0,width - 1]
    int y = std::min(static_cast<int>(v * height), height - 1); // [0, height - 1]
    assert(0 <= x && x < width);
    assert(0 <= y && y < height);
    assert(_raw_data != nullptr);

    auto data = _raw_data + (x + y * width) * components;
    // stb_image: comp 3 are red, green, blue
    // stb_image: comp 4 are red, green, blue, alpha
    // this sets the output to 0xrrggbbaa
    u_int32_t color = (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | ((unsigned)data[0] << 24);
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

// get raw data pointer for metal texture copy
// TODO: this is horrible...
void *image::_getRawData()
{
    return (void *)_raw_data;
}

#endif //  MINITY_SCENE_TYPES_ONLY

} // minity
