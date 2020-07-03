
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <vector>

#include "simpleMath.h"

#define DEG(degree) (degree * 3.14159f / 180)
#define RAD(radians) (radians * 180 / 3.14159f)

struct cam
{
    float fovDegrees = 90.0f;
    vec3 rotation{};
    vec3 translation{};
};

void setPixel(int x, int y)
{
	// TODO: add z and depth check

	// We check if we are actually setting the Pixel inside the Backbuffer
	if (x >= 0 && x < g_SDLWidth)
	{
		if (y >= 0 && y < g_SDLHeight)
		{
			g_SDLBackBuffer[y * g_SDLWidth + x] = 0xffff00ff;
		}
	}
}

void plotLineLow(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    float D = 2*dy - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        setPixel(x, y);
        if (D > 0)
        {
            y += yi;
            D -= 2*dx;
        }
        D += 2*dy;
    }

}

void plotLineHigh(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    float D = 2*dx - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++)
    {
        setPixel(x, y);
        if (D > 0)
        {
            x += xi;
            D -= 2*dy;
        }
        D += 2*dx;
    }

}

void plotLine(int x0, int y0, int x1, int y1)
{
    // std::cout << x0 << y0 << ">" << x1 << y1 << std::endl;
	if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
            plotLineLow(x1, y1, x0, y0);
        else
            plotLineLow(x0, y0, x1, y1);
    }
    else
    {
        if (y0 > y1)
            plotLineHigh(x1, y1, x0, y0);
        else
            plotLineHigh(x0, y0, x1, y1);
    }
}

void drawLine(vec3 from, vec3 to)
{
	int ax, ay, bx, by;

    // if (from.z > 0 || to.z > 0)
    // {
    //     std::cerr << "Z CLIPPING!!!" << std::endl;
    //     return;
    // }

    if (from.x < -1 || from.x > 1
        || from.y < -1 || from.y > 1
        || to.x < -1 ||to.x > 1
        || to.y < -1 ||to.y > 1)
    {
        std::cerr << "XY CLIPPING!!!" << std::endl;
        return;
    }

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    ax = (int)(((from.x + 1.0f) / 2.0f) * (float)g_SDLWidth); //from.x + 10;
	ay = (int)((1.0f - ((from.y + 1.0f) / 2.0f)) * (float)g_SDLHeight); // from.y + 10;
	bx = (int)(((to.x + 1.0f) / 2.0f) * (float)g_SDLWidth); //to.x + 10;
	by = (int)((1.0f - ((to.y + 1.0f) / 2.0f)) * (float)g_SDLHeight); // to.x + 10;
    plotLine(ax, ay, bx, by);
}

void drawMesh(mesh *m, cam *c)
{
	// mat4 scaler = scaleMatrix(0.25f, 0.25f, 0.25f);
    // mat4 scaler = scaleMatrix(1.0f, 1.0f, 1.0f);
    mat4 scaler = scaleMatrix(m->scale.x, m->scale.y, m->scale.z);
    mat4 xRotator = rotateXMatrix(m->rotation.x);
    mat4 yRotator = rotateYMatrix(m->rotation.y);
    mat4 zRotator = rotateZMatrix(m->rotation.z);
    mat4 translator = translateMatrix(m->translation.x, m->translation.y, m->translation.z);

    // order matters: first scale, then translate
    // mat4 worldTransformations = multiplyMat4(translator, scaler);
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);

    // perspective
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    mat4 projector = projectionMatrix(c->fovDegrees, aspectRatio, -0.1f, -400.0f);

    // mat4 cameraYRotator = rotateYMatrix(c->rotation.y);
        // mat4 cameraXRotator = rotateXMatrix(DEG(0)); // 0.4 .. -0.4 (plus is "looking down")
        // TODO: Z feels off???
        // mat4 cameraTranslator = translateMatrix(c->translation.x, c->translation.y, c->translation.z);
        // mat4 cameraMatrix = multiplyMat4(cameraTranslator, cameraYRotator);

    vec3 cameraPos{c->translation.x, c->translation.y, c->translation.z};
    vec3 lookAt{0.0f, 0.0f, 0.0f};
    vec3 up{0.0f, 1.0f, 0.0f};
    // vec3 lookDir = multiplyVec3(lookAt, cameraYRotator);
    // lookAt = v3Add(cameraPos, lookDir);

    // std::cout << "LOOK AT:"  << std::endl;
    // mat4 cameraMatrix = lookAtMatrixRH(vec3{10.0f, 0.0f, 10.0f}, vec3{0.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f});
    // mat4 cameraMatrix = lookAtMatrixRH(vec3{c->translation.x, c->translation.y, c->translation.z}, vec3{0.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f});
    mat4 cameraMatrix = lookAtMatrixRH(cameraPos, lookAt, up);

    // std::cout << "CAMERA MATRIX:"  << std::endl;
    // printMat4(cameraMatrix);

        // mat4 cameraMatrix = multiplyMat4(cameraYRotator, cameraTranslator); // unlikely?
        // mat4 cameraMatrix = multiplyMat4(cameraYRotator, cameraXRotator);
        // cameraMatrix = multiplyMat4(cameraTranslator, cameraMatrix);
        // std::cout << "=======================" << std::endl;
        // printMat4(cameraMatrix);
        // std::cout << "-----------------------" << std::endl;
        // mat4 viewMatrix = inverseMatrixSimple(cameraMatrix);

    mat4 viewMatrix;
    invertRowMajor((float *)cameraMatrix.m, (float *)viewMatrix.m);
    // std::cout << "VIEW MATRIX:"  << std::endl;
    // printMat4(viewMatrix);

        // printMat4(viewMatrix);
        // mat4 viewProjection = multiplyMat4(viewMatrix, projector);
        // mat4 viewProjection = multiplyMat4(projector, viewMatrix); // unlikely?
        // mat4 viewProjection = projector; // debug

    // We loop each Triangle we need to draw
	for(auto triangle : m->tris)
	{
        tri *out, world, view, projected;

        // printTri(triangle, " raw ");

        world = triangle;
        world.vertices[0] = multiplyVec3(triangle.vertices[0], worldTransformations);
        world.vertices[1] = multiplyVec3(triangle.vertices[1], worldTransformations);
        world.vertices[2] = multiplyVec3(triangle.vertices[2], worldTransformations);

        // printTri(world, "world");

        view = world;
        // only camera, no projection
        view.vertices[0] = multiplyVec3(world.vertices[0], viewMatrix);
        view.vertices[1] = multiplyVec3(world.vertices[1], viewMatrix);
        view.vertices[2] = multiplyVec3(world.vertices[2], viewMatrix);

        // printTri(view, " view");

        projected = view;

        // only projection, no camera
        projected.vertices[0] = multiplyVec3(view.vertices[0], projector);
        projected.vertices[1] = multiplyVec3(view.vertices[1], projector);
        projected.vertices[2] = multiplyVec3(view.vertices[2], projector);

        // printTri(projected, " proj");

        // both projection and camera
        // view.vertices[0] = multiplyVec3(world.vertices[0], viewProjection);
        // view.vertices[1] = multiplyVec3(world.vertices[1], viewProjection);
        // view.vertices[2] = multiplyVec3(world.vertices[2], viewProjection);

        // normalise into cartesian space
        projected.vertices[0] = v3Div(projected.vertices[0], projected.vertices[0].w);
        projected.vertices[1] = v3Div(projected.vertices[1], projected.vertices[1].w);
        projected.vertices[2] = v3Div(projected.vertices[2], projected.vertices[2].w);

        // invert x and y (+z) because
        // we are using box-hole camera model
        projected.vertices[0].x *= -1.0f;
        projected.vertices[1].x *= -1.0f;
        projected.vertices[2].x *= -1.0f;
        projected.vertices[0].y *= -1.0f;
        projected.vertices[1].y *= -1.0f;
        projected.vertices[2].y *= -1.0f;
        // view.vertices[0].z *= -1.0f;
        // view.vertices[1].z *= -1.0f;
        // view.vertices[2].z *= -1.0f;

        // printVec3(view.vertices[1]);

        // printTri(projected, "final");

        out = &projected;

        drawLine(out->vertices[0], out->vertices[1]);
		drawLine(out->vertices[1], out->vertices[2]);
		drawLine(out->vertices[2], out->vertices[0]);
    }
}

void printMesh(mesh *m)
{
	for(auto triangle : m->tris)
	{
		std::cout << "v";
		for(int i=0; i < 3; i++)
		{
			vec3 vertice = triangle.vertices[i];
			std::cout << " (" << vertice.x << " " << vertice.y << " "<< vertice.z << ")";
		}
		std::cout << std::endl;
	};
}

// adapted from: https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_olcEngine3D_Part3.cpp
bool loadMeshFromObj(std::string sFilename, mesh *m)
{
    std::ifstream f(sFilename);
    if (!f.is_open())
        return false;

    // Local cache of verts
    std::vector<vec3> verts;

    while (!f.eof())
    {
        char line[128];
        f.getline(line, 128);

        std::strstream s;
        s << line;

        char junk;

        if (line[0] == 'v')
        {
            vec3 v;
            s >> junk >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }

        if (line[0] == 'f')
        {
            int f[3];
            s >> junk >> f[0] >> f[1] >> f[2];
            m->tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
        }
    }
    return true;
}

void clearBuffer()
{
    for (int i = 0; i < g_SDLHeight; ++i)
	{
		for (int j = 0; j < g_SDLWidth; ++j)
		{
			g_SDLBackBuffer[i * g_SDLWidth + j] = 0x000000ff;
        }
    }
}
