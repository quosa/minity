#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL2/SDL.h>
#include <list>

// for vec3 and DEG :-/
#include "simpleMath.h"

struct config
{
    bool drawNormals = false;
    bool drawWireframe = false;
    bool fillTriangles = true;
};
config *g_config = new config();

SDL_Window *g_SDLWindow;
SDL_Renderer *g_SDLRenderer;
SDL_Texture *g_SDLTexture;
SDL_GLContext g_gl_context;
Uint32 *g_SDLBackBuffer;
float *g_DepthBuffer;
int g_SDLWidth;
int g_SDLHeight;
std::list<uint> g_fpsSamples(100, 0);

bool g_show_demo_window = false;
bool g_show_metrics_window = true;
bool g_show_another_window = false;

// Get scaling factor for high-DPI displays
float SDLGetScale() {
  int window_width{0};
  int window_height{0};
  SDL_GetWindowSize(
    g_SDLWindow,
    &window_width, &window_height
  );

  int render_output_width{0};
  int render_output_height{0};
  SDL_GetRendererOutputSize(
    g_SDLRenderer,
    &render_output_width, &render_output_height
  );

  const auto scale_x{
    static_cast<float>(render_output_width) /
      static_cast<float>(window_width)
  };

  return scale_x;
}

void SDLStart(int windowWidth, int windowHeight)
{

    if (SDL_Init(SDL_INIT_VIDEO != 0)) // TODO: | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    fprintf(stdout, "We compiled against SDL version %d.%d.%d ...\n", compiled.major, compiled.minor, compiled.patch);
    fprintf(stdout, "And we are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

    g_SDLWidth = windowWidth;
    g_SDLHeight = windowHeight;

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_ALLOW_HIGHDPI
    );
    g_SDLWindow = SDL_CreateWindow(
        "minity",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
        window_flags);

    if (g_SDLWindow == nullptr)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    // https://github.com/ocornut/imgui/issues/2228
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    g_SDLRenderer = SDL_CreateRenderer(
        g_SDLWindow,
        -1,
        0); // SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED); // SDL_RENDERER_ACCELERATED); // SDL_RENDERER_SOFTWARE); // SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE); // 0);
    if (g_SDLRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(g_SDLRenderer, &info);
    std::cout << "Current SDL_Renderer: " << info.name << std::endl;

    // Set render scale for high DPI displays
    const float scale{SDLGetScale()};
    SDL_RenderSetScale(g_SDLRenderer, scale, scale);


    g_SDLTexture = SDL_CreateTexture(
        g_SDLRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        windowWidth,
        windowHeight);
    // SDL_TEXTUREACCESS_TARGET if we are "copying into the texture" (the bitmap case),
    // otherwise SDL_TEXTUREACCESS_STATIC
    if (g_SDLTexture == nullptr)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    // todo: could use smart pointers instead of globals...
    g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
    g_DepthBuffer = new float[windowWidth * windowHeight];

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(g_SDLWindow, g_SDLRenderer);
    ImGui_ImplSDLRenderer_Init(g_SDLRenderer);
}

void SDLSwapBuffers(/*color_t * backbuffer*/)
{
    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow

    SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
    SDL_RenderClear(g_SDLRenderer);
    SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(g_SDLWindow);
    ImGui::NewFrame();

    if (g_show_demo_window)
        ImGui::ShowDemoWindow(&g_show_demo_window);
    if (g_show_metrics_window)
        ImGui::ShowMetricsWindow(&g_show_metrics_window);
    if (g_show_another_window)
    {
        // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Begin("Another Window", &g_show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            g_show_another_window = false;
        ImGui::End();
    }

    // SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
    // SDL_RenderClear(g_SDLRenderer);
    // SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);

    // Rendering - https://github.com/ocornut/imgui/issues/4264
    ImGui::Render();

    SDL_SetRenderDrawColor(g_SDLRenderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    // SDL_RenderClear(g_SDLRenderer);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(g_SDLRenderer);
}

bool isRunning(vec3 *inputTranslation, vec3 *inputRotation)
{
    SDL_Event sEvent;

    SDL_PollEvent(&sEvent);
    ImGui_ImplSDL2_ProcessEvent(&sEvent);

    switch (sEvent.type)
    {
    case SDL_QUIT:
        return false;
        break;
    case SDL_KEYDOWN:
        switch (sEvent.key.keysym.sym)
        {
        case SDLK_LEFT:
            // std::cerr << " LEFT" << std::endl;
            inputTranslation->x -= 0.5;
            break;
        case SDLK_RIGHT:
            // std::cerr << " RIGHT" << std::endl;
            inputTranslation->x += 0.5;
            break;
        case SDLK_UP:
            // std::cerr << " UP" << std::endl;
            inputTranslation->y += 0.5;
            break;
        case SDLK_DOWN:
            // std::cerr << " DOWN" << std::endl;
            inputTranslation->y -= 0.5;
            break;
        case SDLK_PLUS:
            // std::cerr << " IN" << std::endl;
            inputTranslation->z -= 0.5;
            break;
        case SDLK_MINUS:
            // std::cerr << " OUT" << std::endl;
            inputTranslation->z += 0.5;
            break;
        case SDLK_a:
            // std::cerr << " LookLeft" << std::endl;
            inputRotation->y += deg2rad(5);
            break;
        case SDLK_d:
            // std::cerr << " LookRight" << std::endl;
            inputRotation->y -= deg2rad(5);
            break;
        case SDLK_w:
            // std::cerr << " LookUp" << std::endl;
            inputRotation->x += deg2rad(5);
            break;
        case SDLK_s:
            // std::cerr << " LookDown" << std::endl;
            inputRotation->x -= deg2rad(5);
            break;
        case SDLK_n:
            // std::cerr << " swap (n)ormals" << std::endl;
            g_config->drawNormals = g_config->drawNormals ? false : true;
            break;
        case SDLK_l:
            // std::cerr << " swap wireframe (l)ines" << std::endl;
            g_config->drawWireframe = g_config->drawWireframe ? false : true;
            break;
        case SDLK_f:
            // std::cerr << " swap triangle (f)illing" << std::endl;
            g_config->fillTriangles = g_config->fillTriangles ? false : true;
            break;
        default:
            break;
        }
        break;
    }
    return true;
}

void SDLEnd()
{
    delete[] g_SDLBackBuffer;
    delete[] g_DepthBuffer;

    // UI Cleanup
    // ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    // SDL_GL_DeleteContext(g_gl_context);
    // canvas cleanup
    SDL_DestroyTexture(g_SDLTexture);
    SDL_DestroyRenderer(g_SDLRenderer);
    SDL_DestroyWindow(g_SDLWindow);

    SDL_Quit();
}

void SDLFPSUpdate(uint fps)
{
    g_fpsSamples.push_front(fps);
    g_fpsSamples.pop_back();

    uint avg = 0;
    std::list<uint>::const_iterator it;
    for (it = g_fpsSamples.begin(); it != g_fpsSamples.end(); it++)
        avg += *it;
    avg /= g_fpsSamples.size();

    std::string fpsText = "minity - " + std::to_string(avg) + " fps";
    SDL_SetWindowTitle(g_SDLWindow, fpsText.c_str());
}
