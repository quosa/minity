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
	camera.translation = vec3{0.0f, 0.0f, -2.0f};

	// object.tris = {
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// };

	// object.tris = {
	// 	// bottom-flat triangle
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// 	// top-flat triangle
	// 	{ { {0.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {0.0f, -1.0f, 0.0f} } },
	// };

	// object.tris = {
	// 	{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
	// 	{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
	// };
	// object.translation = vec3{0.0f, -0.0f, -50.0f};

	loadMeshFromObj("models/box.obj", &object);
	// loadMeshFromObj("models/octahedron.obj", &object);
	// object.translation.z = -4.0f;
	// object.scale = vec3{0.5f, 0.5f, 0.5f};
	// object.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	// object.translation = vec3{0.0f, 0.0f, 0.0f};

	// loadMeshFromObj("models/teapot.obj", &object); // check if clockwise???
	// object.scale = vec3{0.5f, 0.5f, 0.5f};
	// object.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	// object.translation = vec3{0.0f, -1.5f, 0.0f};

	// printMesh(&object);
	drawMesh(&object, &camera);
	SDLSwapBuffers();

	auto ft = new jku::frametimer();
	vec3 inputTranslation{};
	vec3 inputRotation{};
	while(isRunning(&inputTranslation, &inputRotation))
    {
		SDL_Delay(20); // some computation budget...
		clearBuffer();

		// object.translation = v3Add(object.translation, inputTranslation);
		// printVec3(object.translation);
		object.rotation.y += DEG(0.5);

		// camera.rotation.y += DEG(0.5); // will overflow eventually
		// printVec3(camera.translation);

		// for camera zoom in/out are opposite
		inputTranslation.z *= 1.0f; // todo: camera is now inverted to positive z-axis (was: -1.0f;)
		camera.translation = v3Add(camera.translation, inputTranslation);
		camera.rotation = v3Add(camera.rotation, inputRotation);
		std::cout << "cam: position " << camera.translation.str()
				<< " rotation " << camera.rotation.str() << std::endl;

		drawMesh(&object, &camera);

	    SDLSwapBuffers();
		SDLFPSUpdate(ft->delta());
		inputTranslation = vec3{};
		inputRotation = vec3{};
    }

    SDLEnd();
}