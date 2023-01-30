#pragma once

#include "../../scene.h"
#include "../../config.h"
#include "../../input.h"
#include "../engine.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_metal.h"
#include <SDL.h>
#include <Foundation/Foundation.hpp> // CA::MetalLayer

#include <simd/simd.h> // vector_uintN

namespace minity
{

class IEngine; // fwd

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

simd::float3 sf3(vec3 &v) {return simd::float3{v.x, v.y, v.z};};
// TODO: color to float4
// simd::float4 sf3(vec3 &v) {return simd::float3{v.x, v.y, v.z};};
void metalEngine::run(scene scene)
{
    auto scale = sf3(scene.model.scale);
    auto position = sf3(scene.model.position);
    auto rotation = sf3(scene.model.rotation);
    // auto colour = scene.model.material.color;
    simd::float4 color{ 1.0f, 1.0f, 1.0f, 1.0f }; // white base-color
    auto metalRenderer = Renderer(layer, scene.model.mesh, scene.model.material.texture);
    while(m_input.handleInput())
    {
        SDL_Delay(20); // some computation budget...
        if (m_input.isKeyPressed(KEY_l))
        {
            g_config->drawWireframe = !g_config->drawWireframe;
        }
        if (m_input.isKeyPressed(KEY_F1))
        {
            g_config->showStatsWindow = !g_config->showStatsWindow;
        }
        scene.model.update(0.1f); // TODO: add frame timer
        position = sf3(scene.model.position);
        scale = sf3(scene.model.scale);
        rotation = sf3(scene.model.rotation);
        metalRenderer.renderModel(position, scale, rotation, color);
    }
}
} // NS minity
