/*
 * ideas from:
 * https://www.github.com/onelonecoder (console 3d engine) and
 * https://github.com/Kayzaks/StupidGL (gamasutra game engine from scratch)
 */

#include <chrono>
#include <iostream>
#include <vector>

#include "sdlHelpers.h"
#include "simpleTimer.h"

struct vec3
{
	float x, y, z;
	/*
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;

	vec3(float x, float y, float z)
	 : x(x), y(y), z(z)
	 {};*/
};

struct tri
{
	vec3 vertices[3];
};

struct mesh
{
	std::vector<tri> tris;
};

struct mat4
{
	float m[4][4] = {{0}};
};

void sGLSetPixel(int x, int y)
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
        sGLSetPixel(x, y);
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
        sGLSetPixel(x, y);
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
    std::cout << x0 << y0 << ">" << x1 << y1 << std::endl;
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
	/*
    int aax, aay, bbx, bby, sx, ex, sy, ey;
    in_out_vertex_t aa, bb;
    aa = g_sGLVertexShader(a);
    bb = g_sGLVertexShader(b);

    // Transform the float coordinates to int's for the screen
    aax = (int)(((aa.x + 1.0f) / 2.0f) * (float)g_sGLBackbufferWidth);
    aay = (int)((1.0f - ((aa.y + 1.0f) / 2.0f)) * (float)g_sGLBackbufferHeight);
    bbx = (int)(((bb.x + 1.0f) / 2.0f) * (float)g_sGLBackbufferWidth);
    bby = (int)((1.0f - ((bb.y + 1.0f) / 2.0f)) * (float)g_sGLBackbufferHeight);
	*/
	int ax, ay, bx, by;
	ax = (int)(((from.x + 1.0f) / 2.0f) * (float)g_SDLWidth); //from.x + 10;
	ay = (int)((1.0f - ((from.y + 1.0f) / 2.0f)) * (float)g_SDLHeight); // from.y + 10;
	bx = (int)(((to.x + 1.0f) / 2.0f) * (float)g_SDLWidth); //to.x + 10;
	by = (int)((1.0f - ((to.y + 1.0f) / 2.0f)) * (float)g_SDLHeight); // to.x + 10;
	plotLine(ax, ay, bx, by);
}
void drawMesh(mesh *m)
{
	// We loop each Triangle we need to draw
	for(auto triangle : m->tris)
	{
        drawLine(triangle.vertices[0], triangle.vertices[1]);
		drawLine(triangle.vertices[1], triangle.vertices[2]);
		drawLine(triangle.vertices[2], triangle.vertices[0]);
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
int main() {
	mesh object;

    std::cout << "f u l l   3 D" << std::endl;

    SDLStart(640, 480);

	object.tris = {
		{ { {0.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.0f, 0.0f} } },
		{ { {0.0f, 0.0f, 0.0f},  {-1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f} } },
	};

	printMesh(&object);
	drawMesh(&object);

	auto ft = new jku::frametimer();

	SDLSwapBuffers();

    while(isRunning())
    {
		SDL_Delay(16); // ~ 60fps minus actual rendering...

	    // SDLSwapBuffers();

		SDLFPSUpdate(ft->delta());
    }

    SDLEnd();
}