#include <algorithm>
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <vector>

#include "simpleMath.h"

struct cam
{
    float fovDegrees = 90.0f;
    vec3 rotation{};
    vec3 translation{};
};

// forward declaration
void plotLine(int x0, int y0, int x1, int y1, u_int32_t rgba_color);

void setPixel(int x, int y, u_int32_t rgba_color)
{
    // TODO: add z and depth check

    // We check if we are actually setting the Pixel inside the Backbuffer
    if (x >= 0 && x < g_SDLWidth)
    {
        if (y >= 0 && y < g_SDLHeight)
        {
            g_SDLBackBuffer[y * g_SDLWidth + x] = rgba_color; // 0xffff00ff;
        }
    }
}

point screenXY(vec3 vertice)
{
    float xx = vertice.x;
    float yy = vertice.y;
    xx = std::max(-1.0f, xx);
    xx = std::min(1.0f, xx);
    yy = std::max(-1.0f, yy);
    yy = std::min(1.0f, yy);

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    return point {
        (int)(((xx + 1.0f) / 2.0f) * (float)g_SDLWidth),
        (int)((1.0f - ((yy + 1.0f) / 2.0f)) * (float)g_SDLHeight)
    };
};

void fillTopFlatTriangle(point v1, point v2, point v3, u_int32_t rgba_color)
{
    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillTopFlatTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    float dx31 = 1.0 * (v3.x - v1.x) / (v3.y - v1.y);
    float dx32 = 1.0 * (v3.x - v2.x) / (v3.y - v2.y);
    float sx = 1.0 * v3.x;
    float ex = 1.0 * v3.x;

    // std::cout << "fTFT: dx31 " << std::to_string(dx31) << ", dx32 " << std::to_string(dx32) << std::endl;

    for (int y = v3.y; y > v2.y; y--)
    {
        plotLine((uint)sx, y, (uint)ex, y, rgba_color);
        sx -= dx31;
        ex -= dx32;
    }
};

void fillBottomFlatTriangle(point v1, point v2, point v3, u_int32_t rgba_color)
{

    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillBottomFlatTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    float dx21 = 1.0 * (v2.x - v1.x) / (v2.y - v1.y);
    float dx31 = 1.0 * (v3.x - v1.x) / (v3.y - v1.y);
    float sx = 1.0 * v1.x;
    float ex = 1.0 * v1.x;

    // std::cout << "fBFT: dx21 " << std::to_string(dx21) << ", dx31 " << std::to_string(dx31) << std::endl;

    for (int y = v1.y; y <= v2.y; y++)
    {
        plotLine((uint)sx, y, (uint)ex, y, rgba_color);
        sx += dx21;
        ex += dx31;
    }
};

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillTriangle(vec3 vertices[3], u_int32_t rgba_color)
{
    point v1 = screenXY(vertices[0]);
    point v2 = screenXY(vertices[1]);
    point v3 = screenXY(vertices[2]);

    if (v1.y > v3.y) pSwap(&v1, &v3);
    if (v1.y > v2.y) pSwap(&v1, &v2);
    if (v2.y > v3.y) pSwap(&v2, &v3);

    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    if (v2.y == v3.y)
    {
        fillBottomFlatTriangle(v1, v2, v3, rgba_color);
    }
    else if (v1.y == v2.y)
    {
        fillTopFlatTriangle(v1, v2, v3, rgba_color);
    }
    else
    {
        // general case
        point v4 {
            (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)),
            v2.y
        };
        fillBottomFlatTriangle(v1, v2, v4, rgba_color);
        fillTopFlatTriangle(v2, v4, v3, rgba_color);
    }
}

void plotLineLow(int x0, int y0, int x1, int y1, u_int32_t rgba_color)
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
        setPixel(x, y, rgba_color);
        if (D > 0)
        {
            y += yi;
            D -= 2*dx;
        }
        D += 2*dy;
    }
}

void plotLineHigh(int x0, int y0, int x1, int y1, u_int32_t rgba_color)
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
        setPixel(x, y, rgba_color);
        if (D > 0)
        {
            x += xi;
            D -= 2*dy;
        }
        D += 2*dx;
    }
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void plotLine(int x0, int y0, int x1, int y1, u_int32_t rgba_color)
{
    // std::cout << "plotline: from (" << std::to_string(x0) << ", "
    //     << std::to_string(y0) << ") to ("
    //     << std::to_string(x1) << ", "
    //     << std::to_string(y1) << ")" << std::endl;
    if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
            plotLineLow(x1, y1, x0, y0, rgba_color);
        else
            plotLineLow(x0, y0, x1, y1, rgba_color);
    }
    else
    {
        if (y0 > y1)
            plotLineHigh(x1, y1, x0, y0, rgba_color);
        else
            plotLineHigh(x0, y0, x1, y1, rgba_color);
    }
}

void drawLine(vec3 from, vec3 to, u_int32_t rgba_color)
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
    plotLine(ax, ay, bx, by, rgba_color);
}

void drawMesh(mesh *m, cam *c)
{
    std::vector<tri> trianglesToSortAndDraw;

    // mat4 scaler = scaleMatrix(0.25f, 0.25f, 0.25f);
    // mat4 scaler = scaleMatrix(1.0f, 1.0f, 1.0f);
    mat4 scaler = scaleMatrix(m->scale.x, m->scale.y, m->scale.z);
    mat4 xRotator = rotateXMatrix(m->rotation.x);
    mat4 yRotator = rotateYMatrix(m->rotation.y);
    mat4 zRotator = rotateZMatrix(m->rotation.z);
    mat4 translator = translateMatrix(m->translation.x, m->translation.y, m->translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);

    // perspective
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    // TODO: near and far field to camera
    mat4 projector = projectionMatrix(c->fovDegrees, aspectRatio, -0.1f, -400.0f);

    vec3 cameraPos{c->translation.x, c->translation.y, c->translation.z};

    // for basic look-at camera
    // vec3 lookAt{0.0f, 0.0f, 0.0f};
    // vec3 up{0.0f, 1.0f, 0.0f};
    // mat4 cameraMatrix = lookAtMatrixRH(cameraPos, lookAt, up);

    // pitch -90 .. 90, yaw 0 ... 360, (both in rad)
    mat4 cameraMatrix = fpsLookAtMatrixRH(cameraPos, c->rotation.x, c->rotation.y);

    mat4 viewMatrix;
    invertRowMajor((float *)cameraMatrix.m, (float *)viewMatrix.m);

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

        // back-face culling - we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for camera rotation
        auto faceNormal = v3Normalize(v3CrossProduct(
            v3Sub(view.vertices[1], view.vertices[0]),
            v3Sub(view.vertices[2], view.vertices[0])
        ));
        auto vCameraRay = v3Normalize(view.vertices[0]); // camera is now at origin
        auto camDot = v3DotProduct(faceNormal, vCameraRay);

        // drawLine(vec3(), faceNormal); // faceNormal sticking out from the face
        // drawLine(vec3(), vCameraRay); // ray from camera to face

        if ( camDot < 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal.str() << " camera:" << vCameraRay.str() << " dot: " << std::to_string(camDot) << std::endl;
            // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
            continue;
        }

        projected = view;
        // only projection, no camera
        projected.vertices[0] = multiplyVec3(view.vertices[0], projector);
        projected.vertices[1] = multiplyVec3(view.vertices[1], projector);
        projected.vertices[2] = multiplyVec3(view.vertices[2], projector);

        // printTri(projected, " proj");

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

        // printTri(projected, "final");

        out = &projected;

        trianglesToSortAndDraw.push_back(*out);

    }
    // sort all to-be-drawn triangles from bact to front
    std::sort(trianglesToSortAndDraw.begin(), trianglesToSortAndDraw.end(), [](tri &t1, tri &t2)
    {
        float z1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z) / 3.0f;
        float z2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z) / 3.0f;
        return z1 > z2;
    });

    // draw all triangles
    for (auto triangle : trianglesToSortAndDraw)
    {
        // order is face first and then wireframe on top
        fillTriangle(triangle.vertices, (u_int32_t)0xccccccff);
        drawLine(triangle.vertices[0], triangle.vertices[1], (u_int32_t)0xeeeeeeff);
        drawLine(triangle.vertices[1], triangle.vertices[2], (u_int32_t)0xeeeeeeff);
        drawLine(triangle.vertices[2], triangle.vertices[0], (u_int32_t)0xeeeeeeff);
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
