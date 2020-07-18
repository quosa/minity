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
	light light;

    std::cout << "minity" << std::endl;

    SDLStart(640, 480);

	camera.fovDegrees = 90.0f;
	camera.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	camera.translation = vec3{0.0f, 0.0f, -2.5f};

	light.rotation = vec3{DEG(45), DEG(-45), DEG(0)};
	light.translation = vec3{10.0f, 10.0f, 10.0f};

	// object.tris = {
	// 	{ { {-0.5f, -0.5f, 0.0f},  {0.0f, 0.5f, 0.0f},  {0.5f, -0.5f, 0.0f} } },
	// };

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

	// loadMeshFromObj("models/box.obj", &object, 0xffff00ff);
	// loadMeshFromObj("models/octahedron.obj", &object);
	// object.translation.z = -4.0f;
	// object.scale = vec3{0.5f, 0.5f, 0.5f};
	// object.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	// object.translation = vec3{0.0f, 0.0f, 0.0f};

	loadMeshFromObj("models/teapot.obj", &object, 0xbbbbbbff);
	// the teapot model is not defined in clockwise order so swap
	// all face vertices
	for(size_t i = 0; i < object.tris.size(); i++)
	{
		auto tmp = object.tris[i].vertices[1];
		object.tris[i].vertices[1] = object.tris[i].vertices[2];
		object.tris[i].vertices[2] = tmp;
	}
	object.scale = vec3{0.5f, 0.5f, 0.5f};
	object.rotation = vec3{DEG(0), DEG(0), DEG(0)};
	object.translation = vec3{0.0f, -1.0f, 0.0f};

	// printMesh(&object);
	clearBuffer();
	drawMesh(&object, &camera, &light);
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

		drawMesh(&object, &camera, &light);

	    SDLSwapBuffers();
		SDLFPSUpdate(ft->delta());
		inputTranslation = vec3{};
		inputRotation = vec3{};
    }

    SDLEnd();
}