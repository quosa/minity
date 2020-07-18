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

struct light
{
    vec3 rotation{};
    vec3 translation{};
};

// forward declaration
void plotLine(int x0, int y0, int x1, int y1, float depth, u_int32_t  rgba_color);

void plotHorizontalLine(int y, int sx,int ex, u_int32_t rgba_color)
{
    if (y < 0 || y >= g_SDLHeight)
        return;

    // Another small performance optimization:
    // fill the horizontal line directly to buffer
    // fillTriangle went from 880ms/7.3%
    // down to 80ms/0.7% with memset_pattern4
    auto ssx = sx > ex ? ex : sx;
    auto eex = sx > ex ? sx : ex;
    auto len = int(eex) - int(ssx);

    ssx = std::max(0, ssx);
    ssx = std::min(ssx, g_SDLWidth);
    eex = std::max(0, eex);
    eex = std::min(eex, g_SDLWidth);

    // std::cout << std::to_string(y)
    //     << " sx " << std::to_string(int(ssx))
    //     << " ex " << std::to_string(int(eex))
    //     << " = " << std::to_string(len)
    //     << std::endl;

    void *p = &g_SDLBackBuffer[y * g_SDLWidth + int(ssx)];
    memset_pattern4(p, &rgba_color, (len + 1) * sizeof(Uint32));
    // TODO: set also g_DepthBuffer
}

void setPixel(int x, int y, float depth, u_int32_t rgba_color)
{
    if (x < 0 || x >= g_SDLWidth || y < 0 || y >= g_SDLHeight)
        return;

    // TODO: add z and depth check

    // We check if we are actually setting the Pixel inside the Backbuffer
    // if (!g_enableDepthBuffer || depth <= g_DepthBuffer[y * g_SDLWidth + x] )
    if ( std::fabsf(g_DepthBuffer[y * g_SDLWidth + x] - depth) <  0.00001f ||  depth < g_DepthBuffer[y * g_SDLWidth + x])
    {
        g_SDLBackBuffer[y * g_SDLWidth + x] = rgba_color; // 0xffff00ff;
        g_DepthBuffer[y * g_SDLWidth + x] = depth;
    }
    else
    {
        // std::cout << "depth clip: depth "
        //     << std::to_string(depth)
        //     << " vs depth buffer"
        //     << std::to_string(g_DepthBuffer[y * g_SDLWidth + x])
        //     << " diff "
        //     << std::to_string(g_DepthBuffer[y * g_SDLWidth + x] - depth)
        //     << std::endl;
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

void fillTopFlatTriangle(point v1, point v2, point v3, float depth, u_int32_t rgba_color)
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
        plotLine((uint)sx, y, (uint)ex, y, depth, rgba_color);
        // plotHorizontalLine(y, int(sx), int(ex), depth, rgba_color);

        sx -= dx31;
        ex -= dx32;
    }
};

void fillBottomFlatTriangle(point v1, point v2, point v3, float depth, u_int32_t rgba_color)
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
        plotLine((uint)sx, y, (uint)ex, y, depth, rgba_color);
        // plotHorizontalLine(y, int(sx), int(ex), depth, rgba_color);
        sx += dx21;
        ex += dx31;
    }
};

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillTriangle(vec3 vertices[3], u_int32_t rgba_color)
{
    float depth = (vertices[0].z + vertices[1].z + vertices[2].z) / 3.0f;
    // std::cout << "triangle depth is " << std::to_string(depth) << std::endl;
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
        fillBottomFlatTriangle(v1, v2, v3, depth, rgba_color);
    }
    else if (v1.y == v2.y)
    {
        fillTopFlatTriangle(v1, v2, v3, depth, rgba_color);
    }
    else
    {
        // general case
        point v4 {
            (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)),
            v2.y
        };
        fillBottomFlatTriangle(v1, v2, v4, depth, rgba_color);
        fillTopFlatTriangle(v2, v4, v3, depth, rgba_color);
    }
}

void plotLineLow(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
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
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            y += yi;
            D -= 2*dx;
        }
        D += 2*dy;
    }
}

void plotLineHigh(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
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
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            x += xi;
            D -= 2*dy;
        }
        D += 2*dx;
    }
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void plotLine(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color)
{
    // std::cout << "plotline: from (" << std::to_string(x0) << ", "
    //     << std::to_string(y0) << ") to ("
    //     << std::to_string(x1) << ", "
    //     << std::to_string(y1) << ")" << std::endl;
    if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
            plotLineLow(x1, y1, x0, y0, depth, rgba_color);
        else
            plotLineLow(x0, y0, x1, y1, depth, rgba_color);
    }
    else
    {
        if (y0 > y1)
            plotLineHigh(x1, y1, x0, y0, depth, rgba_color);
        else
            plotLineHigh(x0, y0, x1, y1, depth, rgba_color);
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

    float depth = (from.z + to.z) / 2.0f;
    // std::cout << "line depth is " << std::to_string(depth) << std::endl;

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
    plotLine(ax, ay, bx, by, depth, rgba_color);
}

void drawMesh(mesh *m, cam *c, light *l)
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

    // light transformations
    mat4 lightXRotator = rotateXMatrix(l->rotation.x);
    mat4 lightYRotator = rotateYMatrix(l->rotation.y);
    mat4 lightZRotator = rotateZMatrix(l->rotation.z);
    mat4 lightTranslator = translateMatrix(l->translation.x, l->translation.y, l->translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 lightTransformations = multiplyMat4(lightYRotator, lightXRotator);
    lightTransformations = multiplyMat4(lightZRotator, lightTransformations);
    lightTransformations = multiplyMat4(lightTranslator, lightTransformations);
    vec3 lightRay = v3Normalize(multiplyVec3(vec3{0.0f, -1.0f, 0.0f}, lightTransformations));
    lightRay.z = 1; // todo: use the light entity for real

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

        if ( camDot < 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal.str() << " camera:" << vCameraRay.str() << " dot: " << std::to_string(camDot) << std::endl;
            // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
            continue;
        }


        // super-simple global Illumination
        vec3 light_direction = v3Normalize(vec3{ 0.0f, -1.0f, -1.0f });
        // How "aligned" are light direction and triangle surface normal?
        float dp = std::max(0.1f, v3DotProduct(light_direction, faceNormal));
        view.color = adjustColor(view.color, dp);

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

        // // draw face normals for debugging
        // auto projectedFaceNormal = v3Div(v3Normalize(v3CrossProduct(
        //     v3Sub(projected.vertices[1], projected.vertices[0]),
        //     v3Sub(projected.vertices[2], projected.vertices[0])
        // )), 10.0f);

        // auto middle = v3Div(
        //     v3Add(projected.vertices[2],v3Add(projected.vertices[1], projected.vertices[0])),
        //     3
        // );
        // // drawLine(vec3(), projectedFaceNormal, 0xffffffff); // faceNormal sticking out from the face
        // drawLine(middle, v3Add(middle, projectedFaceNormal), 0xffffffff); // faceNormal sticking out from the face


        out = &projected;

        fillTriangle(out->vertices, out->color);
        // drawLine(out->vertices[0], out->vertices[1], (u_int32_t)0xeeeeeeff);
        // drawLine(out->vertices[1], out->vertices[2], (u_int32_t)0xeeeeeeff);
        // drawLine(out->vertices[2], out->vertices[0], (u_int32_t)0xeeeeeeff);
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
bool loadMeshFromObj(std::string sFilename, mesh *m, u_int32_t color = 0xccccccff)
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
            m->tris.push_back({ {verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]}, color });
        }
    }
    return true;
}

void clearBuffer()
{
    // first small performance optimization
    // before with -O0 this loop took 1.59s/31.1%
    // after with memset and -O0 this took only 2ms/0%

    // float min_float = std::numeric_limits<float>::lowest();
    // float min_float = 10000.0f;

    // definition: g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
    memset(g_SDLBackBuffer, 0x00, g_SDLWidth * g_SDLHeight * sizeof(Uint32));
    // memset(g_DepthBuffer, 0xff, g_SDLWidth * g_SDLHeight * sizeof(float));
    // memset_pattern4(g_DepthBuffer, &min_float, g_SDLWidth * g_SDLHeight * sizeof(float));

    // for (int i = 0; i < g_SDLHeight; ++i)
    // {
    //     for (int j = 0; j < g_SDLWidth; ++j)
    //     {
    //         g_SDLBackBuffer[i * g_SDLWidth + j] = 0x000000ff;
    //     }
    // }
    for (int i = 0; i < g_SDLHeight; ++i)
    {
        for (int j = 0; j < g_SDLWidth; ++j)
        {
            g_DepthBuffer[i * g_SDLWidth + j] = 10000.0f;
        }
    }

}
