/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

// #define OLD_MAIN_CODE 1
#ifdef OLD_MAIN_CODE
#include <iostream> //cout
#include <memory> // shared_ptr

#include "simpleMath.h" // full implementation here
#include "freezer/old_scene.h" // scene/camera/light/mesh
#include "renderer/software/renderPipeline.h"
#define IMAGEIMPORTER_IMPLEMENTATION
#include "imageImporter.h"
#include "freezer/modelImporter.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

#include <SDL2/SDL.h>
#include "renderer/software/rasterizer.h"

#include "freezer/utils.h" // box, sphere...

// #include "input.h"
#include "minity.h"

const std::string usage = R"(
key bindings:
    wasd keys  - look up/left/down/right
    arrow keys - move camera
    f key      - flat shade triangles
    l key      - draw wireframe
    n key      - draw normals
    p key      - draw point cloud
    x key      - draw axes
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
    minity::imageImporter imgImporter{};
    auto img = imgImporter.load("test/materials/newell_teapot.jpg");

    auto texture = imgImporter.load("test/models/Model_D0606058/CS.JPG", true); // flip
    // auto texture = imgImporter.load("test/materials/texture_uvgrid01.jpg", true); // flip

    // auto boxTexture = imgImporter.load("test/materials/test_image_10x10.png", false); // flip
    // auto boxTexture = imgImporter.load("test/materials/test_image_blue_100x100.png", false); // flip
    // auto boxTexture = imgImporter.load("test/materials/test_image_100x100.png", false); // flip
    auto boxTexture = imgImporter.load("test/materials/texture_uvgrid01.jpg", false); // flip
    // NOTE: fine grid suffers from aliasing / Moire pattern problems due to missing mipmap
    // auto boxTexture = imgImporter.load("test/materials/grid.tga", false); // flip

    // auto tTexture = imgImporter.load("test/materials/grid.tga", true); // flip
    auto tTexture = imgImporter.load("test/materials/test_image_10x10.png", false); // flip
    // auto tTexture = imgImporter.load("test/materials/test_image_100x100.png", true); // flip
    // auto tTexture = imgImporter.load("test/materials/texture_uvgrid01.jpg", false); // flip

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

    minity::old_model test{};
    test.numFaces = 1;
    test.hasNormals = true;
    test.hasTextureCoordinates = true; // false to disable texture mapping
    test.addTexture(boxTexture);
    // clockwise winding order, i.e. center > up > right (left-hand rule!!!)
    test.vertices = {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}; // x, y, z (w=1.0)
    test.normals  = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}; // x, y, z (w=1.0)
    test.textureCoordinates = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}}; // u, v (w ignored)
    test.faces = {{0, 1, 2}, {3, 4, 5}}; // [[v1_idx, v2_idx, v3_idx], [...
    test.scale = vec3{2.0f, 2.0f, 2.0f};
    test.translation = vec3{-1.0f, -1.0f, 2.0f};
    test.rotation = vec3{deg2rad(0), deg2rad(45), deg2rad(0)};
    // test.printModelInfo();
    // test.dumpModel();

    minity::old_model t = minity::square();
    t.addTexture(tTexture);
    t.scale = vec3{2.0f, 2.0f, 2.0f};
    // t.translation = vec3{0.0f, 0.0f, 0.0f};
    // t.rotation = vec3{deg2rad(0), deg2rad(45), deg2rad(0)};
    // t.printModelInfo();
    // t.dumpModel();

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

    // minity::old_scene scene{"teapot", *teapot, camera, light};
    // minity::old_scene scene{"box", *box, camera, light};
    // minity::old_scene scene{"bbox", *bbox, camera, light};
    // minity::old_scene scene{"sphere", *sphere, camera, light};
    // minity::old_scene scene{"male", *male, camera, light};
    minity::old_scene scene{"head", *head, camera, light};
    // minity::old_scene scene{"test", test, camera, light};
    // minity::old_scene scene{"test square", t, camera, light};

    // g_config->renderOnChange = true;
    minity::run(scene);

    minity::shutdown();
}

void newRasterizerScene()
{
    const unsigned int width{640};
    const unsigned int height{480};
    minity::rasterizer rasterizer(width, height);

    minity::init(); // todo: w, h ?

    minity::modelImporter importer{};

    auto teapot = importer.load("test/models/teapot.obj", true); // reverse winding
    teapot->scale = vec3{1.0f, 1.0f, 1.0f};
    teapot->rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    teapot->translation = vec3{0.0f, -1.5f, 0.0f};

    minity::camera camera{};
    minity::light light{};

    camera.fovDegrees = 60.0f;
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    camera.translation = vec3{0.0f, 0.0f, 5.0f};

    // light direction is ignored for now (only global illumination)
    // light.rotation = vec3{deg2rad(45), deg2rad(-45), deg2rad(0)};
    // light is coming from positive z axis
    light.translation = vec3{0.0f, 0.0f, 10.0f};

    minity::old_scene scene{"teapot", *teapot, camera, light};

    g_config->fillTriangles = true;
    g_config->drawWireframe = false;
    g_config->drawNormals = false;
    bool ok = minity::render(scene, rasterizer);
    if (!ok)
    {
        std::cerr << "rendering failed!" << std::endl;
    }

    {
        // SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
        SDL_UpdateTexture(
            g_SDLTexture,
            NULL,
            (void *)rasterizer.getFramebuffer(),
            rasterizer.getViewportWidth() * sizeof(minity::color)
        );
        SDL_RenderClear(g_SDLRenderer);
        SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);
        SDL_RenderPresent(g_SDLRenderer);
    }

    vec3 inputTranslation{};
    vec3 inputRotation{};
    // vec3 zeroVector{};

    bool inputChange{false};
    while (isRunning(&inputTranslation, &inputRotation, &inputChange))
    {
        SDL_Delay(20); // some computation budget...
        inputTranslation = vec3{};
        inputRotation = vec3{};
    }
    minity::shutdown();
}

void newRasterizer()
{
    const unsigned int width{640};
    const unsigned int height{480};
    minity::rasterizer rasterizer(width, height);
    minity::init(); // todo: w, h ?


    // SDLClearBuffers(); // todo: rasterizer.clearBuffers() etc.

    // draw a snap-grid like point cloud
    for (unsigned int x = 10; x < width; x += 10 )
    {
        for (unsigned int y = 10; y < height; y += 10 )
        {
            rasterizer.drawPoint(vec3{(float)x, (float)y, 0.0f}, minity::yellow);
        }
    }
    // rasterizer.drawLine(vec3{}, vec3{640.0f, 480.0f, 0.0f}, minity::red);
    // rasterizer.drawLine(vec3{0.0f, 480.0f, 0.0f}, vec3{640.0f, 0.0f, 0.0f}, minity::green);

    vec3 blueVertices[3]{
        {100.0f, 200.0f, 0.0f},
        {100.0f, 100.0f, 0.0f},
        {200.0f, 200.0f, 0.0f},
    };
    vec3 yellowVertices[3]{
        {70.0f, 220.0f, 1.0f},
        {70.0f, 120.0f, 1.0f},
        {170.0f, 220.0f, 1.0f},
    };
    rasterizer.drawTriangle(blueVertices, minity::blue);
    rasterizer.drawTriangle(yellowVertices, minity::yellow);


    vec3 whiteVertices[3]{
        {400.0f, 400.0f, 1.0f},
        {400.0f, 200.0f, 1.0f},
        {600.0f, 400.0f, -1.0f},
    };
    vec3 greenVertices[3]{
        {400.0f, 400.0f, -1.0f},
        {600.0f, 200.0f, 1.0f},
        {600.0f, 400.0f, 1.0f},
    };
    rasterizer.drawTriangle(whiteVertices, minity::white);
    rasterizer.drawTriangle(greenVertices, minity::green);

    rasterizer.drawLine(vec3{}, vec3{640.0f, 480.0f, 0.0f}, minity::red);
    rasterizer.drawLine(vec3{0.0f, 480.0f, 0.0f}, vec3{640.0f, 0.0f, 0.0f}, minity::green);

    vec3 whiteVertices2[3]{
        {200.0f, 100.0f, 1.0f},
        {400.0f, 100.0f, 1.0f},
        {300.0f, 200.0f, 1.0f},
    };
    vec3 blackVertices[3]{
        {200.0f, 100.0f, 1.00001f},
        {400.0f, 100.0f, 1.00001f},
        {300.0f, 200.0f, 1.00001f},
    };
    rasterizer.drawTriangle(whiteVertices2, minity::white);
    rasterizer.drawTriangle(blackVertices, minity::black);

    // show the drawn buffer
    // SDLSwapBuffers();
    {
        // SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
        SDL_UpdateTexture(
            g_SDLTexture,
            NULL,
            (void *)rasterizer.getFramebuffer(),
            rasterizer.getViewportWidth() * sizeof(minity::color)
        );
        SDL_RenderClear(g_SDLRenderer);
        SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);
        SDL_RenderPresent(g_SDLRenderer);
    }

    vec3 inputTranslation{};
    vec3 inputRotation{};
    // vec3 zeroVector{};

    bool inputChange{false};
    while (isRunning(&inputTranslation, &inputRotation, &inputChange))
    {
        SDL_Delay(20); // some computation budget...
        inputTranslation = vec3{};
        inputRotation = vec3{};
    }
    minity::shutdown();
}

#else


#include "simpleMath.h"
#include "mesh.h"
#include "scene.h"
#include "minity.h"
#define IMAGEIMPORTER_IMPLEMENTATION
#include "imageImporter.h"
#include "meshImporter.h"

#include <iostream> // cout
#include <memory> // shared_ptr

const std::string usage = R"(
key bindings:
    wasd keys  - look up/left/down/right
    arrow keys - move camera
    f key      - flat shade triangles
    l key      - draw wireframe
    n key      - draw normals
    p key      - draw point cloud
    x key      - draw axes
    q key      - quit minity
    r key      - render on change
    F1 key     - show stats window)";


const std::string banner = R"(

 ._ _  o ._  o _|_        |
 | | | | | | |  |_ \/     |____
                   /     /
                        '
)";


void newApi()
{
    minity::minity minity{minity::backend::metal};

    minity::imageImporter imgImporter{};
    // auto texture = imgImporter.load("test/materials/texture_uvgrid01.jpg", true); // flip
    auto texture = imgImporter.load("test/models/Model_D0606058/CS.JPG", true); // flip

    minity::meshImporter meshImporter{};
    // auto mesh = importer.load("test/models/teapot.obj");
    auto mesh = meshImporter.load("test/models/Model_D0606058/head.obj", true); // counter-clockwise winding from 3ds max
    // minity::mesh *mesh = minity::GetSingleFaceMesh();

    minity::material material{minity::yellow, 1.0f, *texture};

    minity::model model{*mesh, material};
    // single face parameters
    // model.scale = vec3{1.0f, 1.0f, 1.0f};
    // model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // model.position = vec3{0.0f, 0.0f, -2.0f};
    // teapot parameters
    // model.scale = vec3{1.0f, 1.0f, 1.0f};
    // model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    // model.position = vec3{0.0f, -0.5f, -4.0f};
    // head parameters
    model.scale = vec3{0.1f, 0.1f, 0.1f};
    model.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};
    model.position = vec3{0.0f, -5.0f, -14.0f};


    auto updateFactory = [](minity::model *self)
    {
        return [self](float timeDelta)
        {
            minity::input &input = minity::input::instance();
            if (input.isKeyDown(minity::KEY_SPACE))
            {
                std::cout << "FIRE!" << std::endl;
            }

            vec3 speed{};

            if (input.isKeyDown(minity::KEY_LEFT))
            {
                speed.x = -0.1f;
            }
            if (input.isKeyDown(minity::KEY_RIGHT))
            {
                speed.x = 0.1f;
            }
            if (input.isKeyDown(minity::KEY_DOWN))
            {
                speed.y = -0.1f;
            }
            if (input.isKeyDown(minity::KEY_UP))
            {
                speed.y = 0.1f;
            }
            if (input.isKeyDown(minity::KEY_MINUS))
            {
                speed.z = -0.1f;
            }
            if (input.isKeyDown(minity::KEY_PLUS))
            {
                speed.z = 0.1f;
            }
            self->position.x += speed.x * timeDelta; // TODO: v3Mul()
            self->position.y += speed.y * timeDelta;
            self->position.z += speed.z * timeDelta;

            // std::cout << "update(" << timeDelta << ") yRot: " << self->rotation.y << " isKeyPressed(minity::KEY_LEFT) " << input.isKeyPressed(minity::KEY_LEFT)<< std::endl;

            float rotationSpeed = 0.1f;
            self->rotation.y += timeDelta * rotationSpeed;
        };
    };
    model.setUpdate(updateFactory);

    // TODO: new camera type
    // minity::camera camera{minity::cameraType::lookAt};
    minity::camera camera{};
    camera.fovDegrees = 50.0f;
    camera.translation = vec3{0.0f, 0.0f, 5.0f};
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};

    // TODO: new light type
    // minity::light light{minity::lightType::directional};
    minity::light light{};
    light.translation = vec3{-1.0f, 1.0f, 10.0f}; // top-left


    minity::scene scene{camera, light, model};

    minity.run(scene);
    minity.shutdown();
}
#endif // OLD MAIN CODE
int main()
{
    std::cout << banner << usage << std::endl;
#ifdef OLD_MAIN_CODE
    newScenario();
    // newRasterizer();
    // newRasterizerScene();
    // metalRendererScenario();
#else
    newApi();
#endif // OLD_MAIN_CODE
}
