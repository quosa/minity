/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <iostream>

// scene already imports imageimporter (until moved)
#define IMAGEIMPORTER_IMPLEMENTATION
#include "scene.h" // scene/camera/light/mesh
#include "renderPipeline.h"
#include "imageImporter.h"
#include "modelImporter.h"


const std::string usage = R"(
key bindings:
    wasd keys  - look up/left/down/right
    arrow keys - move camera
    f key      - flat shade triangles
    l key      - draw wireframe
    n key      - draw normals
    q key      - quit minity
    r key      - render on change
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
    assert(ok); // just to test loading...

    std::shared_ptr<minity::image> texture = std::make_shared<minity::image>();
    ok = texture->load("test/models/Model_D0606058/CS.JPG", true); // flip
    // ok = texture->load("test/materials/texture_uvgrid01.jpg", true); // flip
    assert(ok);

    std::shared_ptr<minity::image> boxTexture = std::make_shared<minity::image>();
    // ok = boxTexture->load("test/materials/test_image_10x10.png", false); // flip
    // ok = boxTexture->load("test/materials/test_image_blue_100x100.png", false); // flip
    // ok = boxTexture->load("test/materials/test_image_100x100.png", false); // flip
    ok = boxTexture->load("test/materials/texture_uvgrid01.jpg", false); // flip

    // NOTE: fine grid suffers from aliasing / Moire pattern problems due to missing mipmap
    // ok = boxTexture->load("test/materials/grid.tga", false); // flip
    assert(ok);

    minity::modelImporter importer{};

    auto teapot = importer.load("test/models/teapot.obj", true); // reverse winding
    teapot->scale = vec3{1.0f, 1.0f, 1.0f};
    teapot->rotation = vec3{deg2rad(30), deg2rad(-30), deg2rad(0)};
    teapot->translation = vec3{0.0f, -1.5f, 0.0f};

    auto box = importer.load("models/box.obj");
    box->scale = vec3{2.0f, 2.0f, 2.0f};
    box->rotation = vec3{deg2rad(0), deg2rad(30), deg2rad(0)};
    box->translation = vec3{0.0f, 0.0f, 1.0f};

    auto bbox = importer.load("models/BlenderBoxZ.obj", true); // counter-clockwise winding from Blender
    bbox->scale = vec3{2.0f, 2.0f, 2.0f};
    bbox->rotation = vec3{deg2rad(0), deg2rad(30), deg2rad(0)};
    bbox->translation = vec3{0.0f, 0.0f, -3.0f};
    // bbox.hasNormals = false;
    // bbox.printModelInfo(true);
    // bbox.printModelInfo();
    // bbox.dumpModel();
    bbox->addTexture(boxTexture);

    auto sphere = importer.load("models/BlenderSmoothSphere.obj", true);  // counter-clockwise winding from Blender
    sphere->scale = vec3{1.0f, 1.0f, 1.0f};
    sphere->rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    sphere->translation = vec3{0.0f, 0.0f, 2.0f};
    // sphere.hasNormals = false;
    sphere->addTexture(boxTexture);

    auto male = importer.load("test/models/MaleLow.obj");
    male->scale = vec3{0.5f, 0.5f, 0.5f};
    male->translation = vec3{0.0f, -5.0f, -6.0f};
    male->rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};

    auto head = importer.load("test/models/Model_D0606058/head.obj", true); // counter-clockwise winding from 3ds max
    head->scale = vec3{0.05f, 0.05f, 0.05f};
    head->translation = vec3{0.0f, -2.5f, -0.8f};
    head->rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    head->addTexture(texture);
    // head.addTexture(boxTexture);
    // head.hasNormals = false;

    std::cout << "image and model imports successful" << std::endl;

    // minity::model test{};
    // test.numFaces = 1;
    // test.hasNormals = true;
    // test.hasTextureCoordinates = true;
    // test.addTexture(boxTexture);
    // // clockwise winding order, i.e. center > up > right (left-hand rule!!!)
    // test.vertices = {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}; // x, y, z (w=1.0)
    // test.normals  = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}; // x, y, z (w=1.0)
    // test.textureCoordinates = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}}; // u, v (w ignored)
    // test.faces = {{0, 1, 2}}; // [[v1_idx, v2_idx, v3_idx], [...
    // test.scale = vec3{1.0f, 1.0f, 1.0f};
    // test.translation = vec3{0.0f, 0.0f, 0.0f};
    // test.rotation = vec3{deg2rad(0), deg2rad(-45), deg2rad(0)};
    // // test.printModelInfo();
    // // test.dumpModel();

    minity::model test{};
    test.numFaces = 1;
    test.hasNormals = true;
    test.hasTextureCoordinates = true; // false to disable texture mapping
    test.addTexture(boxTexture);
    // clockwise winding order, i.e. center > up > right (left-hand rule!!!)
    test.vertices = {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}; // x, y, z (w=1.0)
    test.normals  = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}; // x, y, z (w=1.0)
    test.textureCoordinates = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}}; // u, v (w ignored)
    test.faces = {{0, 1, 2}, {3, 4, 5}}; // [[v1_idx, v2_idx, v3_idx], [...

    // these go a bit outside the viewport
    // test.scale = vec3{4.0f, 4.0f, 4.0f};
    // test.translation = vec3{-1.0f, -2.0f, 2.0f};

    // to make the face fit inside teh viewport
    test.scale = vec3{2.0f, 2.0f, 2.0f};
    test.translation = vec3{-1.0f, -1.0f, 2.0f};

    test.rotation = vec3{deg2rad(0), deg2rad(45), deg2rad(0)};
    // test.printModelInfo();
    // test.dumpModel();

    minity::init();

    minity::camera camera{};
    minity::light light{};

    camera.fovDegrees = 60.0f;
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    camera.translation = vec3{0.0f, 0.0f, 5.0f};

    // light direction is ignored for now (only global illumination)
    // light.rotation = vec3{deg2rad(45), deg2rad(-45), deg2rad(0)};
    // light is coming from positive z axis
    light.translation = vec3{0.0f, 0.0f, 10.0f};

    minity::scene scene{"teapot", *teapot, camera, light};
    // minity::scene scene{"box", box, camera, light};
    // minity::scene scene{"bbox", bbox, camera, light};
    // minity::scene scene{"sphere", sphere, camera, light};
    // minity::scene scene{"male", male, camera, light};
    // minity::scene scene{"head", head, camera, light};
    // minity::scene scene{"test", test, camera, light};

    minity::run(scene);
    // minity::render(scene);
    // vec3 tmp{};
    // while (isRunning(&tmp, &tmp)) SDL_Delay(20);

    minity::shutdown();
}

int main()
{
    std::cout << banner << usage << std::endl;
    newScenario();
}
