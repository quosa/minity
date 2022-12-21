#include <algorithm>
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

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
void plotLine(int x0, int y0, int x1, int y1, float depth, u_int32_t rgba_color);
void setPixel(int x, int y, float depth, u_int32_t rgba_color);

void plotHorizontalLine(point start, point end, u_int32_t rgba_color)
{
    assert(start.y == end.y); // line needs to be horizontal

    // clipping #1 to viewport/projection space
    if (start.y < 0 || start.y >= g_SDLHeight)
        return; // out of screen bounds

    auto y = start.y;
    auto sx = start.x > end.x ? end.x : start.x;
    auto ex = start.x > end.x ? start.x : end.x;
    auto sz = start.x > end.x ? end.z : start.z;
    auto ez = start.x > end.x ? start.z : end.z;
    auto dz = (ez - sz) / (ex - sx);

    assert(ex >= sx);

    // clipping #2 to viewport/projection space
    sx = std::max(0, sx);
    sx = std::min(sx, g_SDLWidth);
    ex = std::max(0, ex);
    ex = std::min(ex, g_SDLWidth);

    // draw horizontal line and use the currect z value for each pixel
    float z = sz;
    for (int x = sx; x <= ex; x++)
    {
        setPixel(x, y, z, rgba_color);
        z += dz;
    }
}

void setPixel(int x, int y, float depth, u_int32_t rgba_color)
{
    // clipping #3 to viewport/projection space
    if (x < 0 || x >= g_SDLWidth || y < 0 || y >= g_SDLHeight)
        return;

    // clipping #4 don't draw pixes that are behind the camera
    // (in view space, the camera is at origin looking towards Z+)
    if (depth < 0.0f)
        return;

    if (depth > 100.0f)
    {
        std::cout << "Pixel far away? (" << x << "," << y << "," << std::to_string(depth) << ")" << std::endl;
    }

    // Z-buffer check: discard deeper pixels (further away from the camera)
    if (depth < g_DepthBuffer[y * g_SDLWidth + x])
    {
        g_SDLBackBuffer[y * g_SDLWidth + x] = rgba_color;
        g_DepthBuffer[y * g_SDLWidth + x] = depth;
    }
}

point screenXY(vec3 vertice)
{
    // vertice must be in normalized device coordinates (NDC)
    // (we choose opengl style: x, y, z inside [-1,1] - Direct3D has z [0,1])

    // screen coordinates are (0, 0) top-left
    // and (width, height) bottom-right

    assert(vertice.x >= -1 && vertice.x <= 1
        && vertice.y >= -1 && vertice.y <= 1
        && vertice.z >= -1 && vertice.z <= 1);

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    // z=-1 -> 0 and z=1 -> 1
    return point{
        (int)(((vertice.x + 1.0f) / 2.0f) * (float)g_SDLWidth),
        (int)((1.0f - ((vertice.y + 1.0f) / 2.0f)) * (float)g_SDLHeight),
        (vertice.z + 1.0f) / 2.0f};
};

void fillTopFlatTriangle(point v1, point v2, point v3, u_int32_t rgba_color)
{
    assert(v1.y <= v2.y && v2.y <= v3.y);

    // std::cout << "fillTopFlatTriangle: v1" << v1.str() << " v2" << v2.str() << " v3" << v3.str() << std::endl;

    float dx31 = 1.0 * (v3.x - v1.x) / (v3.y - v1.y);
    float dx32 = 1.0 * (v3.x - v2.x) / (v3.y - v2.y);
    float sx = 1.0 * v3.x;
    float ex = 1.0 * v3.x;

    float dz31 = 1.0 * (v3.z - v1.z) / (v3.y - v1.y);
    float dz32 = 1.0 * (v3.z - v2.z) / (v3.y - v2.y);
    float sz = v3.z;
    float ez = v3.z;

    // std::cout << "fTFT: dx31 " << std::to_string(dx31) << ", dx32 " << std::to_string(dx32) << std::endl;
    for (int y = v3.y; y > v2.y; y--)
    {
        plotHorizontalLine(
            point{(int)sx, y, sz},
            point{(int)ex, y, ez},
            rgba_color);

        sx -= dx31;
        ex -= dx32;
        sz -= dz31;
        ez -= dz32;
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

    float dz21 = 1.0 * (v2.z - v1.z) / (v2.y - v1.y);
    float dz31 = 1.0 * (v3.z - v1.z) / (v3.y - v1.y);
    float sz = 1.0 * v1.z;
    float ez = 1.0 * v1.z;

    // std::cout << "fBFT: dx21 " << std::to_string(dx21) << ", dx31 " << std::to_string(dx31) << std::endl;

    for (int y = v1.y; y <= v2.y; y++)
    {
        plotHorizontalLine(
            point{(int)sx, y, sz},
            point{(int)ex, y, ez},
            rgba_color);
        sx += dx21;
        ex += dx31;
        sz += dz21;
        ez += dz31;
    }
};

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillTriangle(vec3 vertices[3], u_int32_t rgba_color)
{
    point v1 = screenXY(vertices[0]);
    point v2 = screenXY(vertices[1]);
    point v3 = screenXY(vertices[2]);

    if (v1.y > v3.y)
        pSwap(&v1, &v3);
    if (v1.y > v2.y)
        pSwap(&v1, &v2);
    if (v2.y > v3.y)
        pSwap(&v2, &v3);

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
        // general case, add intermediate point to have
        // one top and bottom flat triangle
        point v4{
            (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)),
            v2.y,
            v1.z + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.z - v1.z)};
        fillBottomFlatTriangle(v1, v2, v4, rgba_color);
        fillTopFlatTriangle(v2, v4, v3, rgba_color);
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
    float D = 2 * dy - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            y += yi;
            D -= 2 * dx;
        }
        D += 2 * dy;
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
    float D = 2 * dx - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++)
    {
        setPixel(x, y, depth, rgba_color);
        if (D > 0)
        {
            x += xi;
            D -= 2 * dy;
        }
        D += 2 * dx;
    }
}

// todo: convert to also calculate the z-coordinate:
//      void plotLine(point from, point to, u_int32_t rgba_color)
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

// convert to use point screenXY(vec3 vertice)
// and remove average depth to get proper z-buffer
void drawLine(vec3 from, vec3 to, u_int32_t rgba_color)
{
    int ax, ay, bx, by;

    // if (from.z > 0 || to.z > 0)
    // {
    //     std::cerr << "Z CLIPPING!!!" << std::endl;
    //     return;
    // }

    if (from.x < -1 || from.x > 1 || from.y < -1 || from.y > 1 || to.x < -1 || to.x > 1 || to.y < -1 || to.y > 1)
    {
        std::cerr << "XY CLIPPING!!!" << std::endl;
        return;
    }

    // todo: calculate per pixel depth!
    float depth = (from.z + to.z) / 2.0f;
    // std::cout << "line depth is " << std::to_string(depth) << std::endl;

    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    ax = (int)(((from.x + 1.0f) / 2.0f) * (float)g_SDLWidth);
    ay = (int)((1.0f - ((from.y + 1.0f) / 2.0f)) * (float)g_SDLHeight);
    bx = (int)(((to.x + 1.0f) / 2.0f) * (float)g_SDLWidth);
    by = (int)((1.0f - ((to.y + 1.0f) / 2.0f)) * (float)g_SDLHeight);
    plotLine(ax, ay, bx, by, depth, rgba_color);
}

void drawMesh(mesh *m, cam *c, light *l)
{
    if (!m->enabled)
        return;

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
    mat4 projector = projectionMatrix(c->fovDegrees, aspectRatio, 0.1f, 400.0f);

    vec3 cameraPos{c->translation.x, c->translation.y, c->translation.z};

    // for basic look-at camera
    vec3 lookAt{0.0f, 0.0f, 0.0f};
    vec3 up{0.0f, 1.0f, 0.0f};
    mat4 cameraMatrix = lookAtMatrixRH(cameraPos, lookAt, up);

    // pitch -90 .. 90, yaw 0 ... 360, (both in rad)
    //mat4 cameraMatrix = fpsLookAtMatrixRH(cameraPos, c->rotation.x, c->rotation.y);

    mat4 viewMatrix = cameraMatrix;

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
    for (auto triangle : m->tris)
    {
        tri *out, world, view, projected;

        // Here triangles are in MODEL/LOCAL SPACE
        // i.e. coordinates coming from the modeling software (and .obj file)

        // printTri(triangle, " raw ");

        world = triangle;
        world.vertices[0] = multiplyVec3(triangle.vertices[0], worldTransformations);
        world.vertices[1] = multiplyVec3(triangle.vertices[1], worldTransformations);
        world.vertices[2] = multiplyVec3(triangle.vertices[2], worldTransformations);

        // Here triangles are in WORLD SPACE
        // i.e. common coordinates for all models in scene

        // printTri(world, "world");

        view = world;
        // only camera, no projection
        view.vertices[0] = multiplyVec3(world.vertices[0], viewMatrix);
        view.vertices[1] = multiplyVec3(world.vertices[1], viewMatrix);
        view.vertices[2] = multiplyVec3(world.vertices[2], viewMatrix);

        // Here triangles are in VIEW SPACE
        // i.e. coordinates looking from camera
        // so a point at world space camera coordinates is (0,0,0)

        // printTri(view, " view");

        // back-face culling - polygons that face away from the camera can be culled
        // we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for camera rotation
        auto faceNormal = v3Normalize(v3CrossProduct(
            v3Sub(view.vertices[1], view.vertices[0]),
            v3Sub(view.vertices[2], view.vertices[0])));
        auto vCameraRay = v3Normalize(view.vertices[0]); // camera is now at origin
        auto camDot = v3DotProduct(faceNormal, vCameraRay);

        if (camDot < 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal.str() << " camera:" << vCameraRay.str() << " dot: " << std::to_string(camDot) << std::endl;
            // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
            continue;
        }

        // super-simple global Illumination
        vec3 light_direction = v3Normalize(vec3{0.0f, -1.0f, -1.0f});
        // How "aligned" are light direction and triangle surface normal?
        float dp = std::max(0.1f, v3DotProduct(light_direction, faceNormal));
        view.color = adjustColor(view.color, dp);

        projected = view;
        // only projection, no camera
        projected.vertices[0] = multiplyVec3(view.vertices[0], projector);
        projected.vertices[1] = multiplyVec3(view.vertices[1], projector);
        projected.vertices[2] = multiplyVec3(view.vertices[2], projector);

        // Here triangles are in CLIP SPACE in homogeneous coordinates
        // https://en.wikipedia.org/wiki/Homogeneous_coordinates

        // normalise into cartesian space
        projected.vertices[0] = v3Div(projected.vertices[0], projected.vertices[0].w);
        projected.vertices[1] = v3Div(projected.vertices[1], projected.vertices[1].w);
        projected.vertices[2] = v3Div(projected.vertices[2], projected.vertices[2].w);

        // Here triangles are in NDC SPACE x, y, z in [-1,1]

        // View volume culling - Geometry outside the view volume can be culled
        int numFacesOutside = 0;
        for (auto &v : projected.vertices)
        {
            if (!(v.x >= -1 && v.x <= 1
             && v.y >= -1 && v.y <= 1
             && v.z >= -1 && v.z <= 1))
             {
                // std::cout << "vertex outside view frustrum: " << v.str() << std::endl;
                numFacesOutside += 1;
             }
        }
        switch (numFacesOutside)
        {
        case 3:
            // if all vertices are outside, this triangle can be discarded
            // std::cout << "discarding face - view volume culling 3 outside " << projected.vertices[0].str() << " " << projected.vertices[1].str() << " " << projected.vertices[2].str() << std::endl;
            continue;
        case 2:
            // TODO: bring the 2 vertices that are outside to screen border
            // std::cout << "discarding face - view volume culling 2 outside " << projected.vertices[0].str() << " " << projected.vertices[1].str() << " " << projected.vertices[2].str() << std::endl;
            continue;
        case 1:
            // TODO: split the base into 2 triangles and change the new vertices to screen border
            // std::cout << "discarding face - view volume culling 1 outside " << projected.vertices[0].str() << " " << projected.vertices[1].str() << " " << projected.vertices[2].str() << std::endl;
            continue;
        case 0:
            // face fully within frustrum - render normally
            break;
        default:
            exit(1);
            break;
        }

        assert(numFacesOutside <= 1);

        // printTri(projected, " proj");

        out = &projected;

        if (g_config->fillTriangles)
        {
            fillTriangle(out->vertices, out->color);
        }

        if (g_config->drawNormals)
        {
            // draw face normals for debugging
            auto projectedFaceNormal = v3Div(v3Normalize(v3CrossProduct(
                                                 v3Sub(projected.vertices[1], projected.vertices[0]),
                                                 v3Sub(projected.vertices[2], projected.vertices[0]))),
                                             10.0f);

            auto middle = v3Div(
                v3Add(projected.vertices[2], v3Add(projected.vertices[1], projected.vertices[0])),
                3);
            drawLine(middle, v3Add(middle, projectedFaceNormal), 0xffffffff); // faceNormal sticking out from the face
        }

        if (g_config->drawWireframe)
        {
            // cheat the z-buffer to actually draw the line
            out->vertices[0].z -= 0.01;
            out->vertices[1].z -= 0.01;
            out->vertices[2].z -= 0.01;
            drawLine(out->vertices[0], out->vertices[1], (u_int32_t)0xeeeeeeff);
            drawLine(out->vertices[1], out->vertices[2], (u_int32_t)0xeeeeeeff);
            drawLine(out->vertices[2], out->vertices[0], (u_int32_t)0xeeeeeeff);
        }
    }
}

void printMesh(mesh *m)
{
    for (auto triangle : m->tris)
    {
        std::cout << "v";
        for (int i = 0; i < 3; i++)
        {
            vec3 vertice = triangle.vertices[i];
            std::cout << " (" << vertice.x << " " << vertice.y << " " << vertice.z << ")";
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
            m->tris.push_back({{verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]}, color});
        }
    }
    return true;
}

void clearBuffer()
{
    // definition: g_SDLBackBuffer = new Uint32[windowWidth * windowHeight];
    memset(g_SDLBackBuffer, 0x00, g_SDLWidth * g_SDLHeight * sizeof(Uint32));

    // g_DepthBuffer = new float[windowWidth * windowHeight];
    // camera is looking towards positize z axis, so z_max is infinity
    float inf = std::numeric_limits<float>::infinity();
    memset_pattern4(g_DepthBuffer, &inf, g_SDLWidth * g_SDLHeight * sizeof(float));
}
