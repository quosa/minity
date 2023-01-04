#pragma once

#define MATH_TYPES_ONLY
#include "simpleMath.h" // mesh etc. for now

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

struct scene
{
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

#endif //  MINITY_SCENE_TYPES_ONLY
} // minity
