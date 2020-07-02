/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <iostream>

#include "sdlHelpers.h"
#include "simpleTimer.h"
#include "simpleGraphics.h"

int main() {
	// mesh object;
    mesh teapot;

    std::cout << "f u l l   3 D" << std::endl;

    SDLStart(640, 480);

	// object.tris = {
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// 	{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
	// };

	// printMesh(&object);
	// drawMesh(&object);

	// loadMeshFromObj("box.obj", &teapot);
	// loadMeshFromObj("octahedron.obj", &teapot);
	loadMeshFromObj("teapot.obj", &teapot);
	teapot.translation.z = -15.0f;
    // printMesh(&teapot);
    drawMesh(&teapot);

	auto ft = new jku::frametimer();

	SDLSwapBuffers();


	while(isRunning())
    {
		SDL_Delay(16); // ~ 60fps minus actual rendering...

		clearBuffer();
		teapot.rotation.y += 0.01;
		drawMesh(&teapot);
	    SDLSwapBuffers();

		SDLFPSUpdate(ft->delta());
    }

    SDLEnd();
}