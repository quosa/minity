/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <iostream> //cout
#include <memory> // shared_ptr

// METAL - INCLUDE IS MESSY SO GET THAT OUT OF THE WAY...
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdtor-name"
// #pragma clang diagnostic ignored "-Werror"
#pragma clang diagnostic ignored "-Wc99-extensions"
#pragma clang diagnostic ignored "-Wc99-designator"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wpedantic"
// ignore the warnings from metal-cpp :-/
// In file included from external/metal-cpp/Foundation/Foundation.hpp:42:
// external/metal-cpp/Foundation/NSSharedPtr.hpp:162:33: error: ISO C++ requires the name after '::~' to be found in the same scope as the name before '::~' [-Werror,-Wdtor-name]
// _NS_INLINE NS::SharedPtr<_Class>::~SharedPtr()
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#pragma clang diagnostic pop


#include "simpleMath.h" // full implementation here
#include "scene.h" // scene/camera/light/mesh
#include "renderPipeline.h"
#define IMAGEIMPORTER_IMPLEMENTATION
#include "imageImporter.h"
#include "modelImporter.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_metal.h"

#include <SDL2/SDL.h>
#include "rasterizer.h"

#include "utils.h" // box, sphere...

#include <simd/simd.h> // vector_uintN
#include "metal_scene.h"
#include "renderer.h"
#include "shader_types.h"


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
    test.scale = vec3{2.0f, 2.0f, 2.0f};
    test.translation = vec3{-1.0f, -1.0f, 2.0f};
    test.rotation = vec3{deg2rad(0), deg2rad(45), deg2rad(0)};
    // test.printModelInfo();
    // test.dumpModel();

    minity::model t = minity::square();
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

    // minity::scene scene{"teapot", *teapot, camera, light};
    // minity::scene scene{"box", *box, camera, light};
    // minity::scene scene{"bbox", *bbox, camera, light};
    // minity::scene scene{"sphere", *sphere, camera, light};
    // minity::scene scene{"male", *male, camera, light};
    minity::scene scene{"head", *head, camera, light};
    // minity::scene scene{"test", test, camera, light};
    // minity::scene scene{"test square", t, camera, light};

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

    minity::scene scene{"teapot", *teapot, camera, light};

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

// utils to print vertices...
std::string sf3(simd::float3 sf3) { return std::to_string(sf3[0]) + " " + std::to_string(sf3[1]) + " " + std::to_string(sf3[2]); };
std::string sf2(simd::float2 sf2) { return std::to_string(sf2[0]) + " " + std::to_string(sf2[1]); };

/**
 * @brief convert a minity::model for use with metal renderer
 *
 * @param path to model obj file
 * @param reverseWinding default true = load as counter clockwise (e.g. from Blender)
 * @return Scene TODO: change to a metal mesh as that is what is in reality returned
 */
inline Scene loadModelAndConvert(const std::string path, bool reverseWinding=true)
{
    minity::modelImporter importer{};
    auto model = importer.load(path, reverseWinding);

    assert(model->faces.size() == (size_t)model->numFaces);
    assert(model->vertices.size() == model->normals.size());
    assert(model->vertices.size() == model->textureCoordinates.size());

    // number of bytes in the vertexData buffer
    size_t vertexDataSize = model->numFaces * 3 * sizeof(VertexData);
    // vertex buffer: 1 * VertexData per vertex * 3 vertices per face * N faces
    VertexData *vertexData = new VertexData[vertexDataSize];
    // number of bytes in the indexData buffer
    size_t indexDataSize = model->numFaces * 3 * sizeof(u_int32_t);
    // index buffer: one 32-bit index per vertex * 3 vertices per face * N faces
    u_int32_t *indexData = new u_int32_t[indexDataSize];

    size_t vIdx = 0;
    for (auto f : model->faces)
    {
        assert(f.size() == 3); // we support only triangles
        for (auto idx : f)
        {
            VertexData tmp{};
            vec3 v = model->vertices[idx];
            vec3 n = model->normals[idx];
            vec2 t = model->textureCoordinates[idx];
            tmp.position = simd::float3{v.x, v.y, v.z};
            tmp.normal = simd::float3{n.x, n.y, n.z};
            tmp.texcoord = simd::float2{t.u, t.v};

            vertexData[vIdx] = tmp;
            indexData[vIdx] = (u_int32_t)vIdx;
            vIdx++;
        }
    }
    return Scene{vertexData, vertexDataSize, indexData, indexDataSize};
}

// https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Dev-Technique/Dev-Technique.html
// https://developer.apple.com/documentation/metal/shader_libraries/building_a_library_with_metal_s_command-line_tools?language=objc
//
// NOTE: -gline-tables-only -frecord-sources add debug info to shader if you want to use xcode metal debugger
//
// xcrun -sdk macosx metal -gline-tables-only -frecord-sources -c renderer.metal -o renderer.air
// xcrun -sdk macosx metallib renderer.air -o renderer.metallib
// xxd -i renderer.metallib renderer_metallib.h
// clang++ -std=c++17 -I /usr/local/include -I ../metal-cpp main.cpp -l SDL2 -framework Foundation -framework QuartzCore -framework Metal && ./a.out
void metalRendererScenario()
{
    const vector_uint2 viewport = {
        640, 480
    };

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); // (void)io;
    ImGui::StyleColorsLight();

    // use a better 18pt font
    const float font_size{18.0f};
    io.Fonts->AddFontFromFileTTF(
        "fonts/SourceCodePro-Regular.ttf",
        font_size
    );
    io.FontDefault = io.Fonts->AddFontFromFileTTF(
        "fonts/SourceCodePro-Regular.ttf",
        font_size
    );

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("SDL Metal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, viewport[0], viewport[1], SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    auto layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(renderer);
    ImGui_ImplMetal_Init(layer->device());
    ImGui_ImplSDL2_InitForMetal(window);

    // Scene bboxScene = loadModelAndConvert("models/BlenderBoxZ.obj");
    // Scene sphereScene = loadModelAndConvert("models/BlenderSmoothSphere.obj");
    Scene headScene = loadModelAndConvert("test/models/Model_D0606058/head.obj");

    // Scene scene = *GetSingleFaceScene(); // only 1 face
    // Scene scene = *GetCubeScene(); // TODO: needs MTL::Winding::WindingCounterClockwise (right-hand winding)
    // Scene &scene = bboxScene;
    // Scene &scene = sphereScene;
    Scene &scene = headScene;

    minity::imageImporter imgImporter{};
    auto headTexture = imgImporter.load("test/models/Model_D0606058/CS.JPG", true); // flip


    assert(headTexture != nullptr); // bc passing as ref
    Renderer metalRenderer = Renderer(layer, scene, *headTexture);

    std::cout << "hello, sdl metal" << std::endl;

    bool quit = false;
    SDL_Event e;

    float angle = 0.0f;

    // cube parameters
    // simd::float3 scale{ 1.0f, 1.0f, 1.0f };
    // simd::float3 position{ 0.0f, 0.0f, -2.0f };

    // Blender box and sphere parameters
    // simd::float3 scale{ 1.0f, 1.0f, 1.0f };
    // simd::float3 position{ 0.0f, 0.0f, -5.0f };

    // head scene parameters
    simd::float3 scale{ 0.1f, 0.1f, 0.1f };
    simd::float3 position{ 0.0f, -5.0f, -12.0f };

    simd::float4 color{ 1.0f, 1.0f, 1.0f, 1.0f }; // white base-color


    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            switch (e.type) {
                case SDL_QUIT: {
                    quit = true;
                } break;
                case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        std::cout << "hello" << std::endl;
                        break;
                    case SDLK_F1:
                        g_config->showStatsWindow = g_config->showStatsWindow ? false : true;
                        break;
                    case SDLK_q:
                        std::cout << "bye" << std::endl;
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
        }


        // cube translation parameters
        angle += 0.01f;

        metalRenderer.renderModel(position, scale, angle, color);

    } // end of rendering loop

    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

int main()
{
    std::cout << banner << usage << std::endl;
    newScenario();
    // newRasterizer();
    // newRasterizerScene();
    metalRendererScenario();
}
