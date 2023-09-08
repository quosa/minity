/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include "simpleMath.h"
#define MESH_UTILS_IMPLEMENTATION
#include "mesh.h"
#include "scene.h"
#include "engine/engine.h"
#include "input.h"
#define IMAGEIMPORTER_IMPLEMENTATION
#include "imageImporter.h"
#include "meshImporter.h"

#include <iostream> // cout
#include <memory> // shared_ptr

const std::string usage = R"(
key bindings:
    wasd keys  - rotate model down/left/up/right
    arrow keys - move model up/left/down/right
    +/- keys   - move model in/out
    f key      - fill/shade triangles
    l key      - draw wireframe
    n key      - draw normals
    p key      - draw point cloud
    x key      - draw axes
    q key      - quit minity
    F1 key     - show stats window)";


const std::string banner = R"(

 ._ _  o ._  o _|_        |
 | | | | | | |  |_ \/     |____
                   /     /
                        '
)";


void runScenario()
{
    auto minity = minity::getEngine(minity::backend::kSoftware);
    // auto minity = minity::getEngine(minity::backend::kMetal);

    minity::imageImporter imgImporter{};
    // auto texture = imgImporter.load("test/materials/texture_uvgrid01.jpg", false); // flip
    // auto texture = imgImporter.load("test/materials/test_image_100x100.png", false); // flip
    // auto texture = imgImporter.load("test/models/Model_D0606058/CS.JPG", true); // flip
    auto texture = imgImporter.load("models/african_head/african_head_diffuse.tga", true); // flip
    // auto texture = imgImporter.load("test/materials/wall_512_3_05.tga", true); // flip
    // auto texture = imgImporter.load("models/from_internet/GroundClay002/GroundClay002_COL_VAR1_1K.jpg", true); // flip

    minity::material material{minity::white, 1.0f, *texture};

    // minity::texture blankTexture {};
    // minity::material material{minity::yellow, 1.0f, blankTexture}; // no texture, just color

    minity::meshImporter meshImporter{};
    // auto mesh = meshImporter.load("test/models/teapot.obj", true); // reverse winding
    // auto mesh = meshImporter.load("test/models/Model_D0606058/head.obj", true); // counter-clockwise winding from 3ds max
    auto mesh = meshImporter.load("models/african_head/african_head.obj", true);
    // auto mesh = meshImporter.load("models/BlenderSmoothSphere.obj", true);  // counter-clockwise winding from Blender
    // auto mesh = minity::getSingleFaceMesh();
    // auto mesh = minity::getSquareMesh();
    // auto mesh = minity::getCubeMesh();

    minity::model model{*mesh, material};
    // single face / square / cube parameters
    // model.scale = vec3{1.0f, 1.0f, 1.0f};
    // model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // model.position = vec3{0.0f, 0.0f, -2.0f};
    // african head parameters
    model.scale = vec3{2.0f, 2.0f, 2.0f};
    model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    model.position = vec3{0.0f, 0.0f, 0.0f};
    // head parameters
    // model.scale = vec3{0.1f, 0.1f, 0.1f};
    // model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // model.position = vec3{0.0f, -5.0f, -14.0f};
    // sphere parameters
    // model.scale = vec3{1.0f, 1.0f, 1.0f};
    // model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // model.position = vec3{0.0f, 0.0f, -2.0f};

    auto updateFactory = [](minity::model *self)
    {
        return [self](float timeDelta)
        {
            minity::input &input = minity::input::instance();
            if (input.isKeyDown(minity::KEY_SPACE))
            {
                std::cout << "FIRE!" << std::endl;
            }

            float movementSpeed = 1.0f;
            auto movementInput = input.getMovementInput();
            self->position = v3Add(self->position, v3Mul(v3Mul(movementInput, movementSpeed), timeDelta));

            auto rotationSpeed = 1.0f;
            auto rotationInput = input.getRotationInput();
            self->rotation = v3Add(self->rotation, v3Mul(v3Mul(rotationInput, rotationSpeed), timeDelta));

            if (g_config->autoRotate)
            {
                float autoRotationSpeed = 0.4f;
                self->rotation.y += timeDelta * autoRotationSpeed;
            }
        };
    };
    model.setUpdate(updateFactory);

    // TODO: add a new camera type
    // minity::camera camera{minity::cameraType::lookAt};
    minity::camera camera{};
    camera.fovDegrees = 50.0f;
    camera.translation = vec3{0.0f, 0.0f, 5.0f};
    // camera.translation = vec3{0.0f, 0.0f, 2.0f}; // for cube mesh
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};

    // TODO: add a new light type
    // minity::light light{minity::lightType::directional};
    minity::light light{};
    light.translation = vec3{-1.0f, 1.0f, 10.0f}; // top-left

    minity::scene scene{camera, light, model};

    minity->run(scene);
    minity->shutdown();
}

int main()
{
    std::cout << banner << usage << std::endl;
    runScenario();
}
