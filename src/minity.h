#pragma once

#include "scene.h"
#include "input.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_metal.h"
#include <SDL.h>

#include <simd/simd.h> // vector_uintN

namespace minity
{

enum backend {software, metal};
class minity
{
public:
    minity(backend renderingBackend);
    // minity(backend renderingBackend) : m_input(input::instance()) {
    //     (void)renderingBackend;
    // };
    // void run(scene scene) {(void)scene;};
    void run(scene scene); //  { (void)scene; /* for (int i = 0; i < 10; i++) scene.model.update(0.01f); */ };
    void shutdown();
private:
    input &m_input;
    SDL_Window *window;
    SDL_Renderer *renderer;
    CA::MetalLayer* layer;
    // Renderer m_metalRenderer;
};

minity::minity(backend renderingBackend) : m_input(input::instance())
{
    assert(renderingBackend == metal);

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

void minity::shutdown()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void minity::run(scene scene)
{
    (void)scene;
    while(m_input.handleInput())
    {
        SDL_Delay(20); // some computation budget...
        if (m_input.isKeyDown(KEY_SPACE))
        {
            std::cout << "FIRE!" << std::endl;
        }
    }
}
} // NS minity
