#include <algorithm>
#include "sdlHelpers.h" // full math
#include "simpleMath.h"
#include "modelImporter.h"

namespace minity
{

const u_int32_t blue = 0x0000ffff;

struct camera
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

void init()
{
    SDLStart(640, 480);
    SDLClearBuffers();
}

enum spaceType
{
    // localSpace
    worldSpace = 0,
    viewSpace = 1,
    clipSpace = 2,
    screenSpace = 3
};

struct renderStats
{
    unsigned long int vertices{0};
    unsigned long int faces{0};
    unsigned long int culled{0};
    unsigned long int clipped{0};
    unsigned long int drawnFaces{0};
    unsigned long int outside{0};
    unsigned long int inside{0};
    unsigned long int drawnPoints{0};
    const unsigned long int bufferSize{
        static_cast<unsigned long int>(g_SDLWidth)
        * static_cast<unsigned long int>(g_SDLHeight)
    };
    friend std::ostream& operator<<(std::ostream& os, const renderStats &stats);
};

// mainly for catch2 to be able to print the assertions
std::ostream& operator<<( std::ostream &os, const renderStats &stats )
{
    float pixelPercentage = 100.0f * stats.inside / (stats.inside + stats.outside);
    os << "render statistics:" << std::endl
       << "  vertices     " << stats.vertices << std::endl
       << "  faces        " << stats.faces << std::endl
       << "  culled       " << stats.culled << std::endl
       << "  drawn faces  " << stats.drawnFaces << std::endl
       << "  clipped      " << stats.clipped << std::endl
       << "  outside      " << stats.outside << std::endl
       << "  inside       " << stats.inside << std::endl
       << "  in : out     " << pixelPercentage << std::endl
    //    << "  in : out     " << static_cast<float>(stats.inside) \
    //         / static_cast<float>(stats.outside) << std::endl // todo: div-by-zero
       << "  drawn points " << stats.drawnPoints << std::endl
       << "  buffer size  " << stats.bufferSize << std::endl;
    return os;
}

// // a.k.a viewport transform
// vec3 screenXY(vec3 vertice)
// {
//     // vertice must be in normalized device coordinates (NDC)
//     // (we choose opengl style: x, y, z inside [-1,1] - Direct3D has z [0,1])

//     // screen coordinates are (0, 0) top-left
//     // and (width, height) bottom-right

//     assert(vertice.x >= -1 && vertice.x <= 1
//         && vertice.y >= -1 && vertice.y <= 1
//         && vertice.z >= -1 && vertice.z <= 1);

//     // Assumes -1 .. +1 box
//     // Screen coordinates: (0,0) is top-left!
//     // (x=-1, y=1) -> (0, 0)
//     // (x=-1, y=-1) -> (0, sHeight)
//     // (x=1, y=1) -> (sWidth, 0)
//     // (x=1, y=-1) -> (sWidth, sHeight)
//     // z=-1 -> 0 and z=1 -> 1

//     // return point{
//     //     (int)(((vertice.x + 1.0f) / 2.0f) * (float)g_SDLWidth),
//     //     (int)((1.0f - ((vertice.y + 1.0f) / 2.0f)) * (float)g_SDLHeight),
//     //     (vertice.z + 1.0f) / 2.0f};
//     return vec3{};
// };


bool render(const minity::model &mdl, const camera &cam, const light &lgt)
{
    renderStats stats{};

    std::cout << "rendering a model with " << mdl.numFaces << " faces, ";
    std::cout << (mdl.hasNormals ? "" : "no ") << "normals and ";
    std::cout << (mdl.hasTextureCoordinates ? "" : "no ") << "texture coordinates";
    std::cout << "." << std::endl;

    mat4 scaler = scaleMatrix(mdl.scale.x, mdl.scale.y, mdl.scale.z);
    mat4 xRotator = rotateXMatrix(mdl.rotation.x);
    mat4 yRotator = rotateYMatrix(mdl.rotation.y);
    mat4 zRotator = rotateZMatrix(mdl.rotation.z);
    mat4 translator = translateMatrix(mdl.translation.x, mdl.translation.y, mdl.translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);

    // perspective
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    // TODO: near and far field to camera
    mat4 projector = projectionMatrix(cam.fovDegrees, aspectRatio, 0.1f, 400.0f);

    vec3 cameraPos{cam.translation.x, cam.translation.y, cam.translation.z};

    // for basic look-at camera
    vec3 lookAt{0.0f, 0.0f, 0.0f};
    vec3 up{0.0f, 1.0f, 0.0f};
    mat4 cameraMatrix = lookAtMatrixRH(cameraPos, lookAt, up);

    mat4 viewMatrix = cameraMatrix;

    // light transformations
    mat4 lightXRotator = rotateXMatrix(lgt.rotation.x);
    mat4 lightYRotator = rotateYMatrix(lgt.rotation.y);
    mat4 lightZRotator = rotateZMatrix(lgt.rotation.z);
    mat4 lightTranslator = translateMatrix(lgt.translation.x, lgt.translation.y, lgt.translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 lightTransformations = multiplyMat4(lightYRotator, lightXRotator);
    lightTransformations = multiplyMat4(lightZRotator, lightTransformations);
    lightTransformations = multiplyMat4(lightTranslator, lightTransformations);
    vec3 lightRay = v3Normalize(multiplyVec3(vec3{0.0f, -1.0f, 0.0f}, lightTransformations));
    lightRay.z = 1; // todo: use the light entity for real

    (void)viewMatrix;
    (void)projector;

    // TODO: start from triangle loop

    for (auto face : mdl.faces)
    {
        stats.faces++;

        u_int32_t faceColor = blue; // simple fallback if no texture

        // SEE: spaceType enum for indices
        // 0 = world, 1 = view, 2 = clip/projected space
        vec3 vects[4][3] = {};
        vec3 norms[4][3] = {};

        // vertex shader (works on vertices)
        for (int idx : face)
        {
            stats.vertices++;
            // Here triangles are in MODEL/LOCAL SPACE
            // i.e. coordinates coming from the modeling software (and .obj file)

            vects[worldSpace][idx % 3] = multiplyVec3(mdl.vertices[idx], worldTransformations);
            if (mdl.hasNormals)
                norms[worldSpace][idx % 3] = multiplyVec3(mdl.normals[idx], worldTransformations);

            // Here triangles are in WORLD SPACE
            // i.e. common coordinates for all models in scene
            // only camera, no projection

            vects[viewSpace][idx % 3] = multiplyVec3(vects[worldSpace][idx % 3], viewMatrix);
            if (mdl.hasNormals)
                norms[viewSpace][idx % 3] = multiplyVec3(norms[worldSpace][idx % 3], viewMatrix);

            // TODO: back-face culling in view space

            // Here triangles are in VIEW SPACE
            // i.e. coordinates looking from camera
            // so a point at world space camera coordinates is (0,0,0)

            // TODO: Global illumination

            vects[clipSpace][idx % 3] = multiplyVec3(vects[viewSpace][idx % 3], projector);
            if (mdl.hasNormals)
                norms[clipSpace][idx % 3] = multiplyVec3(norms[viewSpace][idx % 3], projector);

            // Here triangles are in CLIP SPACE in homogeneous coordinates
            // https://en.wikipedia.org/wiki/Homogeneous_coordinates

            // normalise into cartesian space
            vec3 vClip = vects[clipSpace][idx % 3];

            vects[clipSpace][idx % 3] = v3Div(vClip, vClip.w);
            if (mdl.hasNormals)
            {
                vec3 vNorm = vects[clipSpace][idx % 3];
                norms[clipSpace][idx % 3] = v3Div(vNorm, vNorm.w);
            }

            // Here triangles are in NDC SPACE x, y, z in [-1,1]
            vec3 vertice = vects[clipSpace][idx % 3];
            assert(vertice.x >= -1 && vertice.x <= 1
                && vertice.y >= -1 && vertice.y <= 1
                && vertice.z >= -1 && vertice.z <= 1);


            // TODO: view volume culling (outside frustrum)

            // move all vertices to screen space for rasterization

            // Assumes -1 .. +1 box
            // Screen coordinates: (0,0) is top-left!
            // (x=-1, y=1) -> (0, 0)
            // (x=-1, y=-1) -> (0, sHeight)
            // (x=1, y=1) -> (sWidth, 0)
            // (x=1, y=-1) -> (sWidth, sHeight)
            // z=-1 -> 0 and z=1 -> 1
            vec3 v = multiplyVec3(vects[clipSpace][idx % 3], viewMatrix);
            v.x = (v.x + 1.0f) * static_cast<float>(g_SDLWidth) / 2.0f;
            v.y = (1.0f - ((v.y + 1.0f) / 2.0f)) * static_cast<float>(g_SDLHeight);
            vects[screenSpace][idx % 3] = v;
            // we don't need normals in screenspace???
            // if (mdl.hasNormals)
            //     norms[screenSpace][idx % 3] = multiplyVec3(norms[clipSpace][idx % 3], viewMatrix);

            // Here triangles are all in screen space (0,0) -> (screenWidth, screenHeight)
        } // end of vertex shader (space transformations)

        // Begin rasterization, working in screen space

        // fragment shader (works on triangles)
        vec3 vt1 = vects[screenSpace][0];
        vec3 vt2 = vects[screenSpace][1];
        vec3 vt3 = vects[screenSpace][2];
        if (mdl.hasNormals)
        {
            vec3 nt1 = norms[clipSpace][0];
            vec3 nt2 = norms[clipSpace][1];
            vec3 nt3 = norms[clipSpace][2];
            (void)nt1;
            (void)nt2;
            (void)nt3;
        }
        else
        {
            // calculate a simple face normal

            // Take the view-space as that is not
            // perspective corrected and NDC
            // faces are in clockwise winding order
            // i.e. left-hand rule (thumb faces front)
            // (center > up > right faces front)

            // cross-product a x b follows right-hand rule
            // i.e. a = v2 - v0 and b = v1 - v0
            // so that the normal points front
            vec3 faceNormal = v3Normalize(v3CrossProduct(
                v3Sub(vects[viewSpace][2], vects[viewSpace][0]),
                v3Sub(vects[viewSpace][1], vects[viewSpace][0])));
            // vec3 faceNormal = v3Normalize(v3CrossProduct(
            //     v3Sub(vects[clipSpace][2], vects[clipSpace][0]),
            //     v3Sub(vects[clipSpace][1], vects[clipSpace][0])));


            // back-face culling - polygons that face away from the camera can be culled
            // we do it in view space (=camera coordinates)
            // cannot do it in world space as we have to anyway adjust for camera rotation

            // auto vCameraRay = v3Normalize(vects[clipSpace][0]); // camera is now at origin
            // auto vCameraRay = v3Normalize(vects[viewSpace][0]); // camera is now at origin
            auto vCameraRay = v3Normalize(v3Sub(cam.translation, vects[viewSpace][0])); // camera is now at origin
            auto camDot = v3DotProduct(faceNormal, vCameraRay);

            if (camDot < 0.0f)
            {
                std::cout << "CULLING face normal: " << faceNormal << " camera:" << vCameraRay << " dot: " << std::to_string(camDot) << std::endl << std::endl;
                // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
                stats.culled++;
                continue;
            }

            // light is coming from positive z axis
            vec3 lightDirection = v3Normalize(vec3{1.0f, 0.0f, 1.0f});
            float dp = std::max(0.1f, v3DotProduct(lightDirection, faceNormal));

            for (int idx : face)
                std::cout << std::left << std::setw(35) << mdl.vertices[idx];
            std::cout << std::endl;
            for (int i : {worldSpace, viewSpace, clipSpace, screenSpace})
            {
                for (int j : {0, 1, 2})
                    std::cout << std::left << std::setw(35) << vects[i][j];
                std::cout << std::endl;
            }
            std::cout << "  face normal: " << faceNormal << " dot product: " << dp << std::endl;
            std::cout << "camera normal: " << vCameraRay << " dot: " << std::to_string(camDot) << std::endl << std::endl;
            faceColor = adjustColor(faceColor, dp);
        }

        /* get the bounding box of the triangle */
        int maxX = std::max(vt1.x, std::max(vt2.x, vt3.x));
        int minX = std::min(vt1.x, std::min(vt2.x, vt3.x));
        int maxY = std::max(vt1.y, std::max(vt2.y, vt3.y));
        int minY = std::min(vt1.y, std::min(vt2.y, vt3.y));

        // std::cout << "vt1:" << vt1.str() << " vt2:" << vt2.str() << " vt3:" << vt3.str() <<std::endl;
        // std::cout << "BB:("<<minX<<","<<minY<<") ("<<maxX<<","<<maxY<<")"<<std::endl;

        // Barycentric:
        // https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
        // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html (2d)

/*
void Barycentric(Point p, Point a, Point b, Point c, float &u, float &v, float &w)
{
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}
*/

        // small optimization: pre-calculate common items
        vec3 v0 = v3Sub(vt2, vt1); // b - a
        vec3 v1 = v3Sub(vt3, vt1); // c - a;

        float d00 = v3DotProduct(v0, v0);
        float d01 = v3DotProduct(v0, v1);
        float d11 = v3DotProduct(v1, v1);

        float denom = d00 * d11 - d01 * d01;

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                // Compute barycentric coordinates (u, v, w) for
                // point p with respect to triangle (a, b, c)
                vec3 p  = {static_cast<float>(x), static_cast<float>(y), 0}; // IS THIS 0 CORRECT ???
                // vec3 v0 = v3Sub(vt2, vt1); // b - a
                // vec3 v1 = v3Sub(vt3, vt1); // c - a;
                vec3 v2 = v3Sub(p, vt1); // p - a;
                // float d00 = v3DotProduct(v0, v0);
                // float d01 = v3DotProduct(v0, v1);
                // float d11 = v3DotProduct(v1, v1);
                float d20 = v3DotProduct(v2, v0);
                float d21 = v3DotProduct(v2, v1);
                // float denom = d00 * d11 - d01 * d01;
                float v = (d11 * d20 - d01 * d21) / denom;
                float w = (d00 * d21 - d01 * d20) / denom;
                float u = 1.0f - v - w;

                // TODO z-buffer check

                // setPixel(x, y, color)
                if (u < 0 || v < 0 || w < 0 /*|| zz <= zBuffer[y * image.get_width() + x]*/)
                {
                    stats.outside++;
                    continue; // we're outside the triangle
                }
                stats.inside++;


                // clipping #3 to viewport/projection space
                if (x < 0 || x >= g_SDLWidth || y < 0 || y >= g_SDLHeight)
                {
                    stats.clipped++;
                    continue; // we're outside the viewport
                }


                // D R A W !!!
                // std::cout << "drawing to " << p << std::endl;

                g_SDLBackBuffer[y * g_SDLWidth + x] = faceColor;
                stats.drawnPoints++;

                // TODO: Z-buffer

            }
        }
        stats.drawnFaces++;
    }

    // show the drawn buffer
    SDLSwapBuffers();

    std::cout << stats << std::endl;
    return true;
}

void run()
{
    // SDLFPSUpdate(ft->delta());

    // for now this is just a busy loop
    vec3 inputTranslation{};
    vec3 inputRotation{};
    while (isRunning(&inputTranslation, &inputRotation))
    {
        SDL_Delay(100); // some computation budget...
        inputTranslation = vec3{};
        inputRotation = vec3{};
    }
}

void shutdown()
{
    SDLEnd();
}
} // minity
