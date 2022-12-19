/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <iostream>

#include "sdlHelpers.h"
#include "simpleTimer.h"
#include "simpleGraphics.h"
#include "utils.h"

const std::string usage = R"(minity key bindings:
    wasd keys  - look up/left/down/right
    arrow keys - move camera
    f key      - flat shade triangles
    l key      - draw wireframe
    n key      - draw normals
    q key      - quit minity
    F1 key     - show stats window)";

int main()
{
    mesh object;
    mesh object2;
    cam camera;
    light light;
    float deltaTime = 0.0f;

    std::cout << usage << std::endl;

    SDLStart(640, 480);

    camera.fovDegrees = 60.0f;
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    camera.translation = vec3{0.0f, 0.0f, 5.0f};

    light.rotation = vec3{deg2rad(45), deg2rad(-45), deg2rad(0)};
    // directional light only cares about rotation...
    // light.translation = vec3{10.0f, 10.0f, 10.0f};

    // cartesian coordinates
    const u_int32_t yellow = 0xffff00ff;
    const u_int32_t blue = 0x0000ffff;
    // const u_int32_t green = 0x00ff00ff;
    // const u_int32_t red = 0xff0000ff;
    // object.tris = {
    //     { { {0.0f, 0.0f, 0.0f},  {0.0f, 0.1f, 0.0f},  {2.0f, 0.0f, 0.0f} }, red },
    // 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 2.0f, 0.0f},  {0.1f, 0.0f, 0.0f} }, green },
    //     { { {0.0f, 0.0f, 0.0f},  {0.0f, 0.1f, 0.0f},  {0.0f, 0.0f, 2.0f} }, blue },
    // };

    // object2.tris = {
    // 	{ {  {0.0f, 0.1f, 0.0f},   {0.0f, 0.0f, 0.0f},  {10.0f, 0.0f, 0.0f} } }, // X
    //     { { {0.0f, 10.0f, 0.0f},  {-0.1f, 0.0f, 0.0f},   {0.0f, 0.0f, 0.0f} } }, // Y
    //     { {  {0.0f, 0.1f, 0.0f},  {0.0f, 0.0f, 10.0f},   {0.0f, 0.0f, 0.0f} } }, // Z
    // 	{ { {0.0f, 0.1f, 0.0f},  {10.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f} } }, // X
    //     { { {0.0f, 10.0f, 0.0f},  {0.0f, 0.0f, 0.0f},  {-0.1f, 0.0f, 0.0f} } }, // Y
    //     { { {0.0f, 0.1f, 0.0f},  {0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 10.0f} } }, // Z
    // };

    // object.tris = {
    // 	{ { {-0.5f, -0.5f, -0.0f},  {0.0f, 0.5f, 0.0f},  {0.5f, -0.5f, 0.0f} } },
    // };

    // object.tris = {
    // 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
    // };

    // object.tris = {
    // 	// bottom-flat triangle
    // 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
    // 	// top-flat triangle
    // 	{ { {0.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {0.0f, -1.0f, 0.0f} } },
    // };

    // object.tris = {
    // 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
    // 	{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
    // };
    // object.translation = vec3{0.0f, -0.0f, -50.0f};

    // loadMeshFromObj("models/box.obj", &object, 0xffff00ff);
    // object.translation.y = 5.0f;
    // object.translation.z = -10.0f;

    // loadMeshFromObj("models/octahedron.obj", &object);
    // object.translation.z = -4.0f;
    // object.scale = vec3{0.5f, 0.5f, 0.5f};
    // object.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // object.translation = vec3{0.0f, 0.0f, 0.0f};

    // loadMeshFromObj("models/teapot.obj", &object, 0xbbbbbbff);
    // // the teapot model is not defined in clockwise order so swap
    // // all face vertices
    // for(size_t i = 0; i < object.tris.size(); i++)
    // {
    // 	auto tmp = object.tris[i].vertices[1];
    // 	object.tris[i].vertices[1] = object.tris[i].vertices[2];
    // 	object.tris[i].vertices[2] = tmp;
    // }
    // object.scale = vec3{0.5f, 0.5f, 0.5f};
    // object.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // object.translation = vec3{0.0f, -1.0f, 0.0f};

    sphere(50, 50, yellow, object);
    object.scale = vec3{1.0f, 1.0f, 1.0f};
    object.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    object.translation = vec3{-1.0f, 0.0f, 0.0f};

    sphere(50, 50, blue, object2); // blue is rendered later
    object2.scale = vec3{1.5f, 1.5f, 1.5f};
    object2.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    object2.translation = vec3{1.0f, 0.0f, 0.0f};

    object2.enabled = true;

    // printMesh(&object);
    clearBuffer();
    drawMesh(&object, &camera, &light);
    drawMesh(&object2, &camera, &light);
    SDLSwapBuffers();

    auto ft = new jku::frametimer();
    vec3 inputTranslation{};
    vec3 inputRotation{};
    vec3 zeroVector{};
    while (isRunning(&inputTranslation, &inputRotation))
    {
#if 1 // FPS MODE
        SDL_Delay(20); // some computation budget...
        clearBuffer();

        // object.translation = v3Add(object.translation, inputTranslation);
        // printVec3(object.translation);
        object.rotation.y += deg2rad(deltaTime * 10.0f);
        object2.rotation.y -= deg2rad(deltaTime * 10.0f);

        // camera.rotation.y += deg2rad(0.5); // will overflow eventually
        // printVec3(camera.translation);

        camera.translation = v3Add(camera.translation, inputTranslation);
        camera.rotation = v3Add(camera.rotation, inputRotation);

        if (inputTranslation != zeroVector || inputRotation != zeroVector)
        {

            // SDL_Log("minity: cam pos: %s cam rot: %s \n", camera.translation.str().c_str(), camera.rotation.str().c_str());
            std::cout << "cam: position " << camera.translation.str()
                      << " rotation " << camera.rotation.str() << std::flush << std::endl;
        }
        // std::cout << "cam: position " << camera.translation.str()
        //             << " rotation " << camera.rotation.str() << std::endl;

        drawMesh(&object, &camera, &light);
        drawMesh(&object2, &camera, &light);

        SDLSwapBuffers();
        SDLFPSUpdate(ft->delta());
        deltaTime = ft->deltaTime();
        inputTranslation = vec3{};
        inputRotation = vec3{};
#else // render on input change
        SDL_Delay(100); // busy loop
        if (inputTranslation != zeroVector || inputRotation != zeroVector)
        {
            clearBuffer();
            camera.translation = v3Add(camera.translation, inputTranslation);
            camera.rotation = v3Add(camera.rotation, inputRotation);

            std::cout << "cam: position " << camera.translation.str()
                      << " rotation " << camera.rotation.str() << std::endl;

            drawMesh(&object, &camera, &light);
            drawMesh(&object2, &camera, &light);

            SDLSwapBuffers();
            SDLFPSUpdate(ft->delta());
            inputTranslation = vec3{};
            inputRotation = vec3{};
        }
        deltaTime = ft->deltaTime();
#endif // FPS mode or render-on-input
    }

    SDLEnd();
}
