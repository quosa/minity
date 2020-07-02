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
	mesh object;

    std::cout << "minity" << std::endl;

    SDLStart(640, 480);

	// object.tris = {
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// 	{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
	// };

	// loadMeshFromObj("models/box.obj", &object);
	// loadMeshFromObj("models/octahedron.obj", &object);
	// object.translation.z = -5.0f;

	loadMeshFromObj("models/teapot.obj", &object);
	object.translation = vec3{0.0f, -1.5f, -15.0f};

	// printMesh(&object);
	drawMesh(&object);
	SDLSwapBuffers();

	auto ft = new jku::frametimer();
	while(isRunning())
    {
		SDL_Delay(20); // some computation budget...
		clearBuffer();
		object.rotation.y += 0.01;
		drawMesh(&object);
	    SDLSwapBuffers();
		SDLFPSUpdate(ft->delta());
    }

    SDLEnd();
}