#pragma once

#include "../../scene.h"
#include "../../config.h"
#include "../../input.h"
#include "../frameTimer.h"

#include "../engine_interface.h" // IEngine
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_metal.h"
#include <SDL.h>
#include <Foundation/Foundation.hpp> // CA::MetalLayer

#include <simd/simd.h> // vector_uintN

namespace minity
{

class metalEngine : public IEngine
{
public:
    metalEngine();
    ~metalEngine() = default;
    void run(scene scene);
    void shutdown();
private:
    input &m_input;
    SDL_Window *window;
    SDL_Renderer *renderer;
    CA::MetalLayer* layer;
};

metalEngine::metalEngine() : m_input(input::instance())
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

    window = SDL_CreateWindow("SDL Metal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, viewport[0], viewport[1], SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(renderer);

    ImGui_ImplMetal_Init(layer->device());
    ImGui_ImplSDL2_InitForMetal(window);

    // m_metalRenderer = Renderer(layer, scene, *headTexture);

};

void metalEngine::shutdown()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

/**
 * @brief convert minity::vec3 to simd::float3 for use in metal
 *
 * @param v vertices from model.mesh.vertices
 * @return simd::float3
 */
simd::float3 sf3(const vec3 &v) {return simd::float3{v.x, v.y, v.z};};

/**
 * @brief convert minity::color to simd::float4 for use in metal
 *
 * @param c minity::color from model.material.color
 * @return simd::float4
 */
simd::float4 sf4(const minity::color &c)
{
    float r = static_cast< float >((u_int8_t)(c >> 24)) / 255.0;
    float g = static_cast< float >((u_int8_t)(c >> 16)) / 255.0;
    float b = static_cast< float >((u_int8_t)(c >> 8)) / 255.0;
    float a = static_cast< float >((u_int8_t)(c >> 0)) / 255.0;
    // std::cout << "r: " << r << " g: " << g << " b: " << b << " a: " << a << std::endl;
    return simd::float4{r, g, b, a};
};

void metalEngine::run(scene scene)
{
    float deltaTime = 0.0f;
    auto ft = new minity::frameTimer();

    auto scale = sf3(scene.model.scale);
    auto position = sf3(scene.model.position);
    auto rotation = sf3(scene.model.rotation);
    auto color = sf4(scene.model.material.color);
    auto metalRenderer = Renderer(layer, scene.model.mesh, scene.model.material.texture);
    while(m_input.handleInput())
    {
        SDL_Delay(20); // some computation budget...

        // TODO: unnecessary duplication, move to input or config?
        if (m_input.isKeyPressed(KEY_f))
        {
            g_config->fillTriangles = !g_config->fillTriangles;
        }
        if (m_input.isKeyPressed(KEY_l))
        {
            g_config->drawWireframe = !g_config->drawWireframe;
        }
        if (m_input.isKeyPressed(KEY_n))
        {
            // TODO: not hooked up yet in metal
            g_config->drawNormals = !g_config->drawNormals;
        }
        if (m_input.isKeyPressed(KEY_p))
        {
            // TODO: not hooked up yet in metal
            g_config->drawPointCloud = !g_config->drawPointCloud;
        }
        if (m_input.isKeyPressed(KEY_x))
        {
            // TODO: not hooked up yet in metal
            g_config->drawAxes = !g_config->drawAxes;
        }
        if (m_input.isKeyPressed(KEY_F1))
        {
            g_config->showStatsWindow = !g_config->showStatsWindow;
        }
        if (m_input.isKeyPressed(KEY_r))
        {
            g_config->autoRotate = !g_config->autoRotate;
        }

        scene.model.update(deltaTime);
        position = sf3(scene.model.position);
        scale = sf3(scene.model.scale);
        rotation = sf3(scene.model.rotation);
        metalRenderer.renderModel(position, scale, rotation, color);

        ft->delta(); // update delta time and ignore fps for now
        deltaTime = ft->deltaTime();
    }
}
} // NS minity
