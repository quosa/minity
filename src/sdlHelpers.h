#include <glad/glad.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
// #include <SDL.h>
// #include <SDL_opengl.h>
#include <list>

// for vec3 and DEG :-/
#include "simpleMath.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

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
// SDL_Window *g_imguiWindow;
SDL_GLContext g_gl_context;
// ImGuiIO g_io;
Uint32 *g_SDLBackBuffer;
float *g_DepthBuffer;
int g_SDLWidth;
int g_SDLHeight;
std::list<uint> g_fpsSamples(100, 0);

bool g_show_demo_window = false;
bool g_show_metrics_window = true;
bool g_show_another_window = false;


void SDLStart(int windowWidth, int windowHeight)
{

    if (SDL_Init(SDL_INIT_VIDEO != 0))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    fprintf(stdout, "We compiled against SDL version %d.%d.%d ...\n",compiled.major, compiled.minor, compiled.patch);
    fprintf(stdout, "And we are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

    g_SDLWidth = windowWidth;
    g_SDLHeight = windowHeight;


    // moved after the sdl renderer creation
    // const char* glsl_version = "#version 150";
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // 3
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); // 2

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL
        // | SDL_WINDOW_RESIZABLE
        // | SDL_WINDOW_ALLOW_HIGHDPI
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

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    g_SDLRenderer = SDL_CreateRenderer(
        g_SDLWindow,
        -1,
        SDL_RENDERER_ACCELERATED); // SDL_RENDERER_SOFTWARE); // SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE); // 0);
    if (g_SDLRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

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

    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // SDL_DisplayMode current;
    // SDL_GetCurrentDisplayMode(0, &current);

    g_gl_context = SDL_GL_CreateContext(g_SDLWindow);
    if (g_gl_context == nullptr)
    {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }

    // SDL_GL_MakeCurrent(g_SDLWindow, g_gl_context);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
    {
        std::cerr << "gladLoadGLLoader failed: " << SDL_GetError() << std::endl;
        exit(-1);
    }
    glViewport(0, 0, windowWidth, windowHeight);

    SDL_GL_SetSwapInterval(1); // Enable vsync

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: "
              << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO io = ImGui::GetIO(); (void)io;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(g_SDLWindow, g_gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
}

void SDLSwapBuffers(/*color_t * backbuffer*/)
{
    // SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
    // SDL_RenderClear(g_SDLRenderer);
    // SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(g_SDLWindow);
    ImGui::NewFrame();

    if(g_show_demo_window)
        ImGui::ShowDemoWindow(&g_show_demo_window);
    if(g_show_metrics_window)
        ImGui::ShowMetricsWindow(&g_show_metrics_window);
    if(g_show_another_window)
    {
        ImGui::Begin("Another Window", &g_show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
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
    ImGuiIO io = ImGui::GetIO(); (void)io;
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    // std::cout << (int)io.DisplaySize.x << (int)io.DisplaySize.y << std::endl;
    // glViewport(0, 0, g_SDLWidth, g_SDLHeight);

    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    // glClear(GL_COLOR_BUFFER_BIT);

    // SDL_Renderer
    // SDL_Rect vp;
    // vp.x = vp.y = 0;
    // vp.w = (int) ImGui::GetIO().DisplaySize.x;
    // vp.h = (int) ImGui::GetIO().DisplaySize.y;
    // SDL_RenderSetViewport (g_SDLRenderer, &vp);
    // SDL_SetRenderDrawColor (g_SDLRenderer, clear_color.x * 255.0f, clear_color.y * 255.0f, clear_color.z * 255.0f, clear_color.w * 255.0f);

    // SDL_Rect rct2;
    // rct2.x = rct2.y = 300;
    // rct2.w = rct2.h = 100;
    // SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
    // SDL_RenderClear(g_SDLRenderer);
    // SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);
    // SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, &rct2);

    // ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_SDLWindow);

    // SDL_RenderPresent(g_SDLRenderer);

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(g_gl_context);
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
