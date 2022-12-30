#include <algorithm>
#include "sdlHelpers.h" // full math
#include "simpleMath.h"
#include "modelImporter.h"

namespace minity
{

const bool flatShading = false;

const u_int32_t blue = 0x0000ffff;
const u_int32_t yellow = 0xffff00ff;

void barycentricCoordinatesAt(const vec3 face[3], const vec3 &point, float &u, float &v, float &w);

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
    unsigned long int depth{0};
    const unsigned long int bufferSize{
        static_cast<unsigned long int>(g_SDLWidth)
        * static_cast<unsigned long int>(g_SDLHeight)
    };
    friend std::ostream& operator<<(std::ostream& os, const renderStats &stats);
};

// for std::cout << myRenderStats;
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
       << "  drawn points " << stats.drawnPoints << std::endl
       << "  depth       " << stats.depth << std::endl
       << "  buffer size  " << stats.bufferSize << std::endl;
    return os;
}

bool render(const minity::model &model, const camera &camera, const light &light)
{
    renderStats stats{};

    std::cout << "rendering a model with " << model.numFaces << " faces, ";
    std::cout << (model.hasNormals ? "" : "no ") << "normals and ";
    std::cout << (model.hasTextureCoordinates ? "" : "no ") << "texture coordinates";
    std::cout << "." << std::endl;

    mat4 scaler = scaleMatrix(model.scale.x, model.scale.y, model.scale.z);
    mat4 xRotator = rotateXMatrix(model.rotation.x);
    mat4 yRotator = rotateYMatrix(model.rotation.y);
    mat4 zRotator = rotateZMatrix(model.rotation.z);
    mat4 translator = translateMatrix(model.translation.x, model.translation.y, model.translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 worldTransformations = multiplyMat4(xRotator, scaler);
    worldTransformations = multiplyMat4(yRotator, worldTransformations);
    worldTransformations = multiplyMat4(zRotator, worldTransformations);
    worldTransformations = multiplyMat4(translator, worldTransformations);

    // perspective
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    // TODO: near and far field to camera structure
    mat4 projector = projectionMatrix(camera.fovDegrees, aspectRatio, 0.1f, 400.0f);

    // for basic look-at camera
    vec3 lookAt{0.0f, 0.0f, 0.0f};
    vec3 up{0.0f, 1.0f, 0.0f};
    mat4 cameraMatrix = lookAtMatrixRH( camera.translation, lookAt, up);

    mat4 viewMatrix = cameraMatrix;

    // light transformations
    mat4 lightXRotator = rotateXMatrix(light.rotation.x);
    mat4 lightYRotator = rotateYMatrix(light.rotation.y);
    mat4 lightZRotator = rotateZMatrix(light.rotation.z);
    mat4 lightTranslator = translateMatrix(light.translation.x, light.translation.y, light.translation.z);

    // order matters: scale > rotate > move (=translate)
    mat4 lightTransformations = multiplyMat4(lightYRotator, lightXRotator);
    lightTransformations = multiplyMat4(lightZRotator, lightTransformations);
    lightTransformations = multiplyMat4(lightTranslator, lightTransformations);

    for (auto face : model.faces)
    {
        stats.faces++;

        // TODO: move color to model
        u_int32_t faceColor = yellow; // simple fallback if no texture

        // SEE: spaceType enum for indices
        // 0 = world, 1 = view, 2 = clip/projected space
        vec3 vects[4][3] = {};
        vec3 norms[4][3] = {};

        // vertex shader (works on vertices)
        for (int idx : face)
        {
            stats.vertices++;

            // Here triangles are still in MODEL/LOCAL SPACE
            // i.e. coordinates coming from the modeling software (and .obj file)

            vects[worldSpace][idx % 3] = multiplyVec3(model.vertices[idx], worldTransformations);
            if (model.hasNormals)
            {
                // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
                auto mn = model.normals[idx];
                mn.w = 0;
                norms[worldSpace][idx % 3] = multiplyVec3(mn, transposeMat4(invertMat4(worldTransformations)));
            }


            // Here triangles are in WORLD SPACE
            // i.e. common coordinates for all models in scene
            // only camera, no projection

            vects[viewSpace][idx % 3] = multiplyVec3(vects[worldSpace][idx % 3], viewMatrix);
            if (model.hasNormals)
            {
                // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
                norms[viewSpace][idx % 3] = multiplyVec3(norms[worldSpace][idx % 3], transposeMat4(invertMat4(viewMatrix)));
            }

            // Here triangles are in VIEW SPACE
            // i.e. coordinates looking from camera
            // so a point at world space camera coordinates is (0,0,0)

            vects[clipSpace][idx % 3] = multiplyVec3(vects[viewSpace][idx % 3], projector);
            if (model.hasNormals)
            {
                // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
                norms[clipSpace][idx % 3] = multiplyVec3(norms[viewSpace][idx % 3], transposeMat4(invertMat4(projector)));
            }

            // Here triangles are in CLIP SPACE in homogeneous coordinates
            // https://en.wikipedia.org/wiki/Homogeneous_coordinates

            // normalise into cartesian space
            vec3 vClip = vects[clipSpace][idx % 3];
            vects[clipSpace][idx % 3] = v3Div(vClip, vClip.w);
            if (model.hasNormals)
            {
                vec3 nClip = norms[clipSpace][idx % 3]; // ???
                norms[clipSpace][idx % 3] = v3Div(nClip, nClip.w); // ???
            }

            // Here triangles are in NDC SPACE x, y, z in [-1,1]

            // model can be outside of the view area
            // vec3 vertice = vects[clipSpace][idx % 3];
            // assert(vertice.x >= -1 && vertice.x <= 1
            //     && vertice.y >= -1 && vertice.y <= 1
            //     && vertice.z >= -1 && vertice.z <= 1);


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
            // z is retained as -1 .. 1 for z-buffering
            vects[screenSpace][idx % 3] = v;

            // we don't need normals in screenspace???
            // if (model.hasNormals)
            //     norms[screenSpace][idx % 3] = multiplyVec3(norms[clipSpace][idx % 3], viewMatrix);

            // Here triangles are all in screen space (0,0) -> (screenWidth, screenHeight)

        } // end of vertex shader (space transformations)

        // Begin rasterization, working in screen space

        // fragment shader (works on triangles)
        vec3 vt1 = vects[screenSpace][0];
        vec3 vt2 = vects[screenSpace][1];
        vec3 vt3 = vects[screenSpace][2];
        vec3 nt1{};
        vec3 nt2{};
        vec3 nt3{};
        if (model.hasNormals)
        {
            nt1 = norms[viewSpace][0];
            nt2 = norms[viewSpace][1];
            nt3 = norms[viewSpace][2];
        }

        // TODO: if the model does not have texture and texture coordinates
        // calculate a simple face normal

        // Take the view-space as that is not
        // perspective corrected

        // faces are in clockwise winding order
        // i.e. left-hand rule (thumb faces front)
        // (center > up > right faces front)

        // cross-product a x b follows right-hand rule
        // i.e. a = v2 - v0 and b = v1 - v0
        // so that the normal points front
        vec3 faceNormal = v3Normalize(v3CrossProduct(
            v3Sub(vects[viewSpace][2], vects[viewSpace][0]),
            v3Sub(vects[viewSpace][1], vects[viewSpace][0])));



        // Print rendering debug info from each face at each stage
        // char n[4] = {'w', 'v', 'c', 's'};
        // std::cout << "face #" << stats.faces << std::endl;
        // for (spaceType s : {worldSpace, viewSpace, clipSpace, screenSpace})
        // {
        //     std::cout << n[s];
        //     for (int i : {0, 1, 2})
        //         std::cout << " v" << i << " " << std::setw(35)<< vects[s][i];
        //     std::cout << " | ";
        //     for (int i : {0, 1, 2})
        //         std::cout << " n" << i << " " << std::setw(35) << norms[s][i];
        //     std::cout << std::endl;
        // }



        // TODO: decide if we should take the view or clip space face normal
        // if change: remember to adjust faceNormal and vCameraRay!
        // if I take clip space, flat shading becomes really smooth?
        // with viewSpace, I get teh flat triangles as expected.

        // back-face culling - polygons that face away from the camera can be culled
        // we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for possible camera rotation

        auto vCameraRay = v3Normalize(v3Sub(vec3{0}, vects[viewSpace][0])); // in view space camera is at origin, so pick one vertice and calculate direction towards it
        auto camDot = v3DotProduct(faceNormal, vCameraRay);

        if (camDot < 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal << " camera:" << vCameraRay << " dot: " << std::to_string(camDot) << std::endl << std::endl;
            // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
            stats.culled++;
            continue;
        }

        vec3 lightDirection = v3Normalize(light.translation);

        if (!model.hasNormals)
        {
            // FLAT SHADING
            float dp = std::max(0.1f, v3DotProduct(lightDirection, faceNormal));
            faceColor = adjustColor(faceColor, dp);
            std::cout << "face normal " << faceNormal << " dp is " << dp << std::endl;
            printColor(faceColor);
        }

        /* get the bounding box of the triangle */
        int maxX = std::max(vt1.x, std::max(vt2.x, vt3.x));
        int minX = std::min(vt1.x, std::min(vt2.x, vt3.x));
        int maxY = std::max(vt1.y, std::max(vt2.y, vt3.y));
        int minY = std::min(vt1.y, std::min(vt2.y, vt3.y));

        // std::cout << "vt1:" << vt1.str() << " vt2:" << vt2.str() << " vt3:" << vt3.str() <<std::endl;
        // std::cout << "BB:("<<minX<<","<<minY<<") ("<<maxX<<","<<maxY<<")"<<std::endl;

        // barycentric coordinates
        float u{0};
        float v{0};
        float w{0};

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {

                vec3 point  = {static_cast<float>(x), static_cast<float>(y), 0}; // IS THIS 0 CORRECT ???
                barycentricCoordinatesAt(vects[screenSpace], point, u, v, w);

                if (u < 0 || v < 0 || w < 0)
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

                // z-buffer (depth) check
                // get the z value for this point using the barymetric coordinates:
                float z = vt1.z * u + vt2.z * v + vt3.z * w;


                // std::cout << x << " " << y << " " << z << " vs (" << g_DepthBuffer[y * g_SDLWidth + x] << ")" << std::endl;
                // TODO: < or <= here? Do we draw the new pixel if x, y, z are the same?
                if (z < g_DepthBuffer[y * g_SDLWidth + x])
                {

                    u_int32_t c = faceColor;
                    // calculate normal at (x, y) and adjust face color
                    // Phong shading?
                    if (model.hasNormals)
                    {
                        vec3 vn =  v3Normalize(v3Add(v3Add(v3Mul(nt1, u), v3Mul(nt2, v)), v3Mul(nt3, w)));
                        float dp = std::max(0.1f, v3DotProduct(lightDirection, vn));
                        c = adjustColor(c, dp);
                        // std::cout << "barycentric normal " << vn << " dp is " << dp << std::endl;
                        // printColor(c);
                    }

                    // std::cout << "drawing to " << p << std::endl;
                    g_SDLBackBuffer[y * g_SDLWidth + x] = c;
                    g_DepthBuffer[y * g_SDLWidth + x] = z;
                    stats.drawnPoints++;
                }
                else
                {
                    stats.depth++;
                }
            }
        }
        stats.drawnFaces++;
    }

    // show the drawn buffer
    SDLSwapBuffers();

    std::cout << stats << std::endl;
    return true;
}

void barycentricCoordinatesAt(const vec3 face[3], const vec3 &point, float &u, float &v, float &w)
{
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

    // Compute barycentric coordinates (u, v, w) for
    // point p with respect to triangle (a, b, c)
    vec3 v0 = v3Sub(face[1], face[0]); // b - a
    vec3 v1 = v3Sub(face[2], face[0]); // c - a;
    vec3 v2 = v3Sub(point, face[0]); // p - a;
    float d00 = v3DotProduct(v0, v0);
    float d01 = v3DotProduct(v0, v1);
    float d11 = v3DotProduct(v1, v1);
    float d20 = v3DotProduct(v2, v0);
    float d21 = v3DotProduct(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
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
