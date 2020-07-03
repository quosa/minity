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
	cam camera;

    std::cout << "minity" << std::endl;

    SDLStart(640, 480);

	camera.fovDegrees = 90.0f;
	camera.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	camera.translation = vec3{0.0f, 0.0f, 10.0f};

	// object.tris = {
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// 	{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
	// };
	// object.translation = vec3{0.0f, -0.0f, -50.0f};

	loadMeshFromObj("models/box.obj", &object);
	// loadMeshFromObj("models/octahedron.obj", &object);
	// object.translation.z = -4.0f;
	object.scale = vec3{1.0f, 1.0f, 1.0f};
	object.rotation = vec3{DEG(180), DEG(180), DEG(180)};
	object.translation = vec3{0.0f, 0.0f, 10.0f};

	// loadMeshFromObj("models/teapot.obj", &object);
	// object.translation = vec3{0.0f, -1.5f, -15.0f};

	// printMesh(&object);
	drawMesh(&object, &camera);
	SDLSwapBuffers();

	auto ft = new jku::frametimer();
	vec3 input{};
	while(isRunning(&input))
    {
		SDL_Delay(20); // some computation budget...
		clearBuffer();

		object.translation = v3Add(object.translation, input);
		// printVec3(object.translation);

		// camera.rotation.y += DEG(0.1); // will overflow eventually
		// camera.translation.x -= 0.01;

		drawMesh(&object, &camera);

		// screen coordinate test
		// drawLine(vec3{-1.0f, -1.0f, 0.0f}, vec3{1.0f, 1.0f, 0.0f});
		// drawLine(vec3{-1.0f, 0.0f, 0.0f}, vec3{0.0f, -1.0f, 0.0f});
		// drawLine(vec3{0.0f, 1.0f, 0.0f}, vec3{0.0f, -1.0f, 0.0f});
		// drawLine(vec3{0.0f, 0.0f, 0.0f}, vec3{1.0f, -1.0f, 0.0f});
		// drawLine(vec3{-1.0f, 0.5f, 0.0f}, vec3{1.1f, 0.5f, 0.0f}); // is clipped!

	    SDLSwapBuffers();
		SDLFPSUpdate(ft->delta());
		input = vec3{};
    }

    SDLEnd();
}