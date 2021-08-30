#include <SDL.h>
#include <list>

// for vec3 and DEG :-/
#include "simpleMath.h"

struct config
{
    bool drawNormals = false;
    bool drawWireframe = false;
    bool fillTriangles = true;
};
config * g_config = new config();

SDL_Window * g_SDLWindow;
SDL_Renderer * g_SDLRenderer;
SDL_Texture * g_SDLTexture;
Uint32 * g_SDLBackBuffer;
float * g_DepthBuffer;
int g_SDLWidth;
int g_SDLHeight;
std::list<uint> g_fpsSamples(100,0);

void SDLStart(int windowWidth, int windowHeight)
{
	SDL_Init(SDL_INIT_VIDEO);

	g_SDLWidth = windowWidth;
	g_SDLHeight = windowHeight;

	g_SDLWindow = SDL_CreateWindow(
        "minity",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
        0);

	g_SDLRenderer = SDL_CreateRenderer(
        g_SDLWindow,
        -1,
        0); // SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE); // 0);
        // only hints???

	// pick the format - for me rgba8888 makes most sense
    // from https://wiki.libsdl.org/SDL_PixelFormatEnum
    // g_SDLTexture = SDL_CreateTexture(g_SDLRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, windowWidth, windowHeight);
	// g_SDLTexture = SDL_CreateTexture(g_SDLRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, windowWidth, windowHeight);

    g_SDLTexture = SDL_CreateTexture(
        g_SDLRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        windowWidth,
        windowHeight);

        // SDL_TEXTUREACCESS_TARGET if we are "copying into the texture" (the bitmap case),
        // otherwise SDL_TEXTUREACCESS_STATIC

	// todo: could use smart pointers instead of globals...
    g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
    g_DepthBuffer = new float[windowWidth * windowHeight];
}

void SDLSwapBuffers(/*color_t * backbuffer*/)
{
	SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
	SDL_RenderClear(g_SDLRenderer);
	SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);
	SDL_RenderPresent(g_SDLRenderer);
}

bool isRunning(vec3 *inputTranslation, vec3 *inputRotation)
{
	SDL_Event sEvent;

	SDL_PollEvent(&sEvent);

	switch (sEvent.type)
	{
	case SDL_QUIT:

		return false;

		break;
    case SDL_KEYDOWN:
        switch( sEvent.key.keysym.sym )
        {
            case SDLK_LEFT:
                std::cerr << " LEFT";
                inputTranslation->x += 0.5; // todo: camera is now inverted
                break;
            case SDLK_RIGHT:
                std::cerr << " RIGHT";
                inputTranslation->x -= 0.5; // todo: camera is now inverted
                break;
            case SDLK_UP:
                std::cerr << " UP";
                inputTranslation->y -= 0.5; // todo: camera is now inverted
                break;
            case SDLK_DOWN:
                std::cerr << " DOWN";
                inputTranslation->y += 0.5; // todo: camera is now inverted
                break;
            case SDLK_PLUS:
                std::cerr << " IN";
                inputTranslation->z += 1;
                break;
            case SDLK_MINUS:
                std::cerr << " OUT";
                inputTranslation->z -= 1;
                break;
            case SDLK_a:
                std::cerr << " LookLeft";
                inputRotation->y -= DEG(5);
                break;
            case SDLK_d:
                std::cerr << " LookRight";
                inputRotation->y += DEG(5);
                break;
            case SDLK_w:
                std::cerr << " LookUp";
                inputRotation->x += DEG(5);
                break;
            case SDLK_s:
                std::cerr << " LookDown";
                inputRotation->x -= DEG(5);
                break;
            case SDLK_n:
                std::cerr << " swap (n)ormals";
                g_config->drawNormals = g_config->drawNormals ? false : true;
                break;
            case SDLK_l:
                std::cerr << " swap wireframe (l)ines";
                g_config->drawWireframe = g_config->drawWireframe ? false : true;
                break;
            case SDLK_f:
                std::cerr << " swap triangle (f)illing";
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
    for(it = g_fpsSamples.begin(); it != g_fpsSamples.end(); it++) avg += *it;
    avg /= g_fpsSamples.size();

    std::string fpsText = "minity - " + std::to_string(avg) + " fps";
	SDL_SetWindowTitle(g_SDLWindow, fpsText.c_str());
}
