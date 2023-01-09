#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <SDL.h>
#include <list>

// for vec3 and deg2rad :-/
#define MATH_TYPES_ONLY
#include "simpleMath.h"

struct config
{
    bool drawNormals = false; // n key
    bool drawWireframe = false; // l key
    bool fillTriangles = true; // f key
    bool showStatsWindow = false; // F1 key
    bool renderOnChange = false; // r key
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

// stats
std::list<uint> g_fpsSamples(100, 0);
uint g_avg{0};
std::string g_stats{};

// runtime config info
std::string g_SDLVersion;
std::string g_SDLLinkedVersion;
std::string g_SDLRendererType;
std::string g_ImGuiVersion;

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
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

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
        SDL_RENDERER_SOFTWARE);
    if (g_SDLRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

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
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // TODO: docking would be nice...
    // io.ConfigFlags |=  ImGuiWindowFlags_AlwaysAutoResize;  // Resize every window to its content every frame

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // use a better font

    // Get proper display scaling for high DPI displays
    const float font_scaling_factor{scale};

    // Font size will be 18pt
    const float font_size{18.0F * font_scaling_factor};

    // Load font and set as default with proper scaling
    io.Fonts->AddFontFromFileTTF(
        "fonts/SourceCodePro-Regular.ttf",
        font_size
    );
    io.FontDefault = io.Fonts->AddFontFromFileTTF(
        "fonts/SourceCodePro-Regular.ttf",
        font_size
    );
    io.FontGlobalScale = 1.0F / font_scaling_factor;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(g_SDLWindow, g_SDLRenderer);
    ImGui_ImplSDLRenderer_Init(g_SDLRenderer);

    // store SDL and ImGui versions for stats screen
    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    g_SDLVersion = std::to_string(compiled.major)
        + "." + std::to_string(compiled.minor)
        + "." + std::to_string(compiled.patch);

    g_SDLLinkedVersion = std::to_string(linked.major)
        + "." + std::to_string(linked.minor)
        + "." + std::to_string(linked.patch);

    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(g_SDLRenderer, &rendererInfo);
    g_SDLRendererType = rendererInfo.name;

    g_ImGuiVersion = std::to_string(IMGUI_VERSION_NUM);
}

void SDLClearBuffers()
{
    // definition: g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
    memset(g_SDLBackBuffer, 0x00, g_SDLWidth * g_SDLHeight * sizeof(Uint32));

    // g_DepthBuffer = new float[windowWidth * windowHeight];
    // camera is looking towards positize z axis, so z_max is infinity
    float inf = std::numeric_limits<float>::infinity();
    memset_pattern4(g_DepthBuffer, &inf, g_SDLWidth * g_SDLHeight * sizeof(float));

    // after calculating the 1/z, we initialize to 0 and keep higher values (smaller z)
    // float negInf = -std::numeric_limits<float>::infinity();
    // float invNegInf = 1.0f / negInf;
    // memset_pattern4(g_DepthBuffer, &invNegInf, g_SDLWidth * g_SDLHeight * sizeof(float));
}

void SDLSwapBuffers(/*color_t * backbuffer*/)
{
    SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
    SDL_RenderClear(g_SDLRenderer);
    SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);


    if (g_config->showStatsWindow)
    {
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame(g_SDLWindow);
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::Begin("Minity Stats Window", &g_config->showStatsWindow, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Minity is running at %u fps using %s renderer.", g_avg, g_SDLRendererType.c_str());
        ImGui::Text("%s", g_stats.c_str());
        ImGui::Text("SDL compiled %s, linked %s.", g_SDLVersion.c_str(), g_SDLLinkedVersion.c_str());
        ImGui::Text("ImGui %s.", g_ImGuiVersion.c_str());
        ImGui::End();
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    }
    SDL_RenderPresent(g_SDLRenderer);
}

bool isRunning(vec3 *inputTranslation, vec3 *inputRotation, bool *inputChange)
{
    SDL_Event sEvent;

    *inputChange = false;
    while (SDL_PollEvent(&sEvent))
    {
        ImGui_ImplSDL2_ProcessEvent(&sEvent);

        switch (sEvent.type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_KEYDOWN:
            *inputChange = true;
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
            case SDLK_r:
                // std::cerr << " (r)ender on change" << std::endl;
                g_config->renderOnChange = g_config->renderOnChange ? false : true;
                break;
            case SDLK_F1:
                // std::cerr << " show/hide stats window" << std::endl;
                g_config->showStatsWindow = g_config->showStatsWindow ? false : true;
                break;
            case SDLK_q:
                return false;
                break;
            default:
                break;
            }
            break;
        }

    }
    return true;
}

void SDLEnd()
{
    delete[] g_SDLBackBuffer;
    delete[] g_DepthBuffer;

    // UI Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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

    g_avg = 0;
    std::list<uint>::const_iterator it;
    for (it = g_fpsSamples.begin(); it != g_fpsSamples.end(); it++)
        g_avg += *it;
    g_avg /= g_fpsSamples.size();

    std::string fpsText = "minity - " + std::to_string(g_avg) + " fps";
    SDL_SetWindowTitle(g_SDLWindow, fpsText.c_str());
}
