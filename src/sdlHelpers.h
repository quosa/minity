#include <SDL.h>
#include <list>

SDL_Window * g_SDLWindow;
SDL_Renderer * g_SDLRenderer;
SDL_Texture * g_SDLTexture;
Uint32 * g_SDLBackBuffer;
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

	g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
}

void SDLSwapBuffers(/*color_t * backbuffer*/)
{
	SDL_UpdateTexture(g_SDLTexture, NULL, g_SDLBackBuffer, g_SDLWidth * sizeof(Uint32));
	SDL_RenderClear(g_SDLRenderer);
	SDL_RenderCopy(g_SDLRenderer, g_SDLTexture, NULL, NULL);
	SDL_RenderPresent(g_SDLRenderer);
}

bool isRunning()
{
	SDL_Event sEvent;

	SDL_PollEvent(&sEvent);

	switch (sEvent.type)
	{
	case SDL_QUIT:

		return false;

		break;
	}

	return true;
}

void SDLEnd()
{
	delete[] g_SDLBackBuffer;

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
