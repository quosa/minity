/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <iostream>

// #include "originalScenario.h"
#include "renderPipeline.h"
#include "imageImporter.h" // math imports only
#include "modelImporter.h"

const std::string usage = R"(
key bindings:
    wasd keys  - look up/left/down/right
    arrow keys - move camera
    f key      - flat shade triangles
    l key      - draw wireframe
    n key      - draw normals
    q key      - quit minity
    F1 key     - show stats window)";

const std::string banner = R"(

 ._ _  o ._  o _|_        |
 | | | | | | |  |_ \/     |____
                   /     /
                        '
)";

void newScenario()
{
    bool ok = false;

    minity::image img{};
    ok = img.load("test/materials/newell_teapot.jpg");
    assert(ok);

    minity::model teapot{};
    ok = teapot.load("test/models/teapot.obj", true); // reverse winding
    assert(ok);
    teapot.scale = vec3{1.0f, 1.0f, 1.0f};
    teapot.rotation = vec3{deg2rad(30), deg2rad(-30), deg2rad(0)};
    teapot.translation = vec3{0.0f, -1.5f, 0.0f};

    minity::model box{};
    ok = box.load("models/box.obj");
    assert(ok);
    box.scale = vec3{2.0f, 2.0f, 2.0f};
    box.rotation = vec3{deg2rad(0), deg2rad(-30), deg2rad(0)};
    box.translation = vec3{0.0f, 0.0f, 1.0f};

    minity::model male{};
    ok = male.load("test/models/MaleLow.obj"); // , true); // reverse winding
    assert(ok);
    male.scale = vec3{0.5f, 0.5f, 0.5f};
    male.translation = vec3{0.0f, -5.0f, -6.0f};
    male.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};

    std::cout << "image and model import successful" << std::endl;

    minity::init();

    minity::camera camera{};
    minity::light light{};

    camera.fovDegrees = 60.0f;
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    camera.translation = vec3{0.0f, 0.0f, 5.0f};

    // light direction is ignored for now (only global illumination)
    // light.rotation = vec3{deg2rad(45), deg2rad(-45), deg2rad(0)};
    // light is coming from positive z axis
    light.translation = vec3{0.0f, 0.0f, 1.0f}; // vec3{1.0f, 0.0f, 1.0f}


    // draw the model just once
    // todo: move all to a scene that is rendered
    // ok = minity::render(teapot, camera, light);
    // ok = minity::render(box, camera, light);
    ok = minity::render(male, camera, light);
    if (!ok)
    {
        std::cerr << "Trouble rendering model" << std::endl;
    }

    minity::run();

    minity::shutdown();
}

int main()
{
    std::cout << banner << usage << std::endl;
    // originalScenario();
    newScenario();
}
