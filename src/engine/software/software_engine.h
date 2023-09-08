#include <algorithm>
#include <cassert>
#include <sstream>

#include "../engine_interface.h" // IEngine
#include "../../input.h"

#include "../../simpleMath.h"
#include "sdlHelpers.h" // full math
#include "../frameTimer.h"
#include "rasterizer.h"
#include "stats.h"

namespace minity
{

class softwareEngine : public IEngine
{
public:
    softwareEngine();
    ~softwareEngine() = default;
    void run(scene scene);
    void shutdown();
private:
    input &m_input;
};


inline vec3 &toScreenXY(vec3 &point, unsigned int screenWidth, unsigned int screenHeight)
{
    point.x = (point.x + 1.0f) * static_cast<float>(screenWidth) / 2.0f;
    point.y = (1.0f - ((point.y + 1.0f) / 2.0f)) * static_cast<float>(screenHeight);
    return point;
}

softwareEngine::softwareEngine() : m_input(input::instance())
{
    SDLStart(640, 480);
    SDLClearBuffers();
};

enum spaceType
{
    // localSpace
    worldSpace = 0,
    viewSpace = 1,
    clipSpace = 2,
    screenSpace = 3,
    ndcCoordinates = 4
};

// TODO: make matrices come from model getters
// TODO: figure out if we need to add some encapsulation for vects/norms/texc
void processVertices(
    minity::model &model,
    const int faceIndex, const int idx,
    vec3 (&vects)[5][3], vec3 (&norms)[5][3], vec2 (&texc)[3],
    const mat4 &modelMatrix, const mat4 &viewMatrix, const mat4 &projectionMatrix,
    const mat4 &inverseModelMatrix, const mat4 &inverseViewMatrix, const mat4 &inverseProjectionMatrix,
    renderStats &stats)
{
    stats.vertices++;

    // Here triangles are still in MODEL/LOCAL SPACE
    // i.e. coordinates coming from the modeling software (and .obj file)

    auto modelIndex = model.mesh.indexData[faceIndex * 3 + idx];
    // std::cout << "index: " << modelIndex << " vertex: " << model.mesh.vertexData[modelIndex].position << std::endl;

    vects[worldSpace][idx % 3] = multiplyVec3(model.mesh.vertexData[modelIndex].position, modelMatrix);
    // if (model.hasNormals)
    // {
    // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
    auto mn = model.mesh.vertexData[modelIndex].normal;
    mn.w = 0;
    norms[worldSpace][idx % 3] = multiplyVec3(mn, transposeMat4(inverseModelMatrix));
    // }
    // if (model.hasTextureCoordinates)
    // {
    texc[idx % 3] = model.mesh.vertexData[modelIndex].texcoord;
    // }

    // Here triangles are in WORLD SPACE
    // i.e. common coordinates for all models in scene
    // only camera, no projection

    vects[viewSpace][idx % 3] = multiplyVec3(vects[worldSpace][idx % 3], viewMatrix);
    // if (model.hasNormals)
    // {
    // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
    norms[viewSpace][idx % 3] = multiplyVec3(norms[worldSpace][idx % 3], transposeMat4(inverseViewMatrix));
    // }

    // Here triangles are in VIEW SPACE
    // i.e. coordinates looking from camera
    // so a point at world space camera coordinates is (0,0,0)

    vects[clipSpace][idx % 3] = multiplyVec3(vects[viewSpace][idx % 3], projectionMatrix);
    // if (model.hasNormals)
    // {
    // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
    norms[clipSpace][idx % 3] = multiplyVec3(norms[viewSpace][idx % 3], transposeMat4(inverseProjectionMatrix));
    // }

    // Here triangles are in CLIP SPACE in homogeneous coordinates
    // https://en.wikipedia.org/wiki/Homogeneous_coordinates

    // TODO: maybe we should move the ndc and screen conversion to the rasterizer
    // as that would align better with the normal rendering pipeline where
    // the vertex shader moves vertices from model to clip space?

    // normalise into cartesian space
    vec3 vClip = vects[clipSpace][idx % 3];
    vects[ndcCoordinates][idx % 3] = v3Div(vClip, vClip.w);
    // if (model.hasNormals)
    // {
    vec3 nClip = norms[clipSpace][idx % 3]; // ???
    norms[ndcCoordinates][idx % 3] = v3Div(nClip, nClip.w); // ???
    // }

    // Here triangles are in NDC SPACE x, y, z in [-1,1]

    // move all vertices to screen space for rasterization
    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    // z=-1 -> 0 and z=1 -> 1

    vec3 v = vects[ndcCoordinates][idx % 3];
    v.x = (v.x + 1.0f) * static_cast<float>(g_SDLWidth) / 2.0f;
    v.y = (1.0f - ((v.y + 1.0f) / 2.0f)) * static_cast<float>(g_SDLHeight);
    // z is retained as -1 .. 1 for z-buffering
    vects[screenSpace][idx % 3] = v;

    // we don't need normals in screenspace???
    // if (model.hasNormals)
    //     norms[screenSpace][idx % 3] = multiplyVec3(norms[clipSpace][idx % 3], viewMatrix);

    // Here triangles are all in screen space (0,0) -> (screenWidth, screenHeight)
};

/**
 * @brief clip a triangle to within the clip-space volume
 *
 * TODO: add new faces if clipped (and adjust the return value)
 *
 * @param ndcVertices
 * @param stats
 * @return true if the entire triangle is within the clip-space volume
 * @return false if there are any vertices outside the clip-space volume
 */
bool clippingFunction(vec3 (&ndcVertices)[3], renderStats &stats)
{
    // CLIPPING faces to within the clip-space volume
    int numFacesOutside = 0;
    for (auto &v : ndcVertices) // vects[ndcCoordinates])
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
        // std::cout << "CLIPPING - discarding face, 3 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
        stats.vfCulled++;
        return false;
    case 2:
        // TODO: bring the 2 vertices that are outside to screen border
        // std::cout << "CLIPPING - discarding face, 2 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
        stats.vfCulled++;
        return false;
    case 1:
        // TODO: split the base into 2 triangles and change the new vertices to screen border
        // std::cout << "CLIPPING - discarding face, 1 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
        stats.vfCulled++;
        return false;
    case 0:
        // face fully within frustrum - render normally
        break;
    default:
        std::cerr << "unknown number of faces putside camera frustrum " << numFacesOutside << " - exiting" << std::endl;
        exit(1);
        break;
    }

    assert(numFacesOutside <= 1);
    return true;
};

/**
 * @brief back face culling
 *
 * @param viewVertices
 * @param stats
 * @return true if face is forwardfacing and should be drawn
 * @return false if face is backwardfacing and should be culled
 */
bool cullingFunction(vec3 (&viewVertices)[3], renderStats &stats)
{
        // SEE: https://en.wikipedia.org/wiki/Back-face_culling
        // This is view space culling, we can also do (NDC) clip space culling later

        // back-face culling - polygons that face away from the camera can be culled
        // we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for possible camera rotation

        // faces are in clockwise winding order
        // i.e. left-hand rule (thumb faces front)
        // (center > up > right faces front)

        // cross-product a x b follows right-hand rule
        // i.e. a = v2 - v0 and b = v1 - v0
        // so that the normal points front
        vec3 faceNormal = v3Normalize(v3CrossProduct(
            v3Sub(viewVertices[2], viewVertices[0]),
            v3Sub(viewVertices[1], viewVertices[0])));

        auto vCameraRay = v3Normalize(v3Sub(vec3{0}, viewVertices[0])); // in view space camera is at origin, so pick one vertice and calculate direction towards it
        auto faceDotCamera = v3DotProduct(faceNormal, vCameraRay);

        if (faceDotCamera <= 0.0f)
        {
            stats.bfCulled++;
            return false;
        }
        return true;
}

bool render(minity::scene scene, minity::rasterizer &rasterizer)
{
    renderStats stats{};
    vec3 zero3{};
    vec2 zero2{};

    minity::model &model = scene.model;
    minity::camera &camera = scene.camera;
    minity::light &light = scene.light;

    rasterizer.clearBuffers();

    // rough check if we have normals, texture coordinates and texture
    bool hasNormals = model.mesh.vertexData[0].normal != zero3;
    bool hasTextureCoordinates = model.mesh.vertexData[0].texcoord != zero2;
    bool hasTexture = model.material.texture.width > 0 && model.material.texture.height > 0;

    // all objects in same coordinates
    // LOCAL/MODEL SPACE TO WORLD SPACE
    mat4 modelMatrix = model.getModelTransformMatrix();
    mat4 inverseModelMatrix = invertMat4(modelMatrix);

    // look from camera
    // WORLD SPACE TO VIEW SPACE
    mat4 viewMatrix = camera.getCameraMatrix();
    mat4 inverseViewMatrix = invertMat4(viewMatrix);

    // apply perspective to camera view
    // VIEW SPACE TO CLIP SPACE
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    // TODO: add near and far field to minity camera structure
    // TODO: projection matrix should be pre-calculated as it changes only if camera fov changes
    mat4 projectionMatrix = perspectiveProjectionMatrix(camera.fovDegrees, aspectRatio, 0.1f, 400.0f);
    mat4 inverseProjectionMatrix = invertMat4(projectionMatrix);


    mat4 lightMatrix = light.getLightTransformationMatrix();
    (void)lightMatrix; // TODO: use the light for diffusion

    // for (auto face : model.faces)
    assert(model.mesh.indexData.size() % 3 == 0);
    size_t numFaces = model.mesh.indexData.size() / 3;
    size_t faceIndex = 0;
    while(faceIndex < numFaces)
    {
        stats.faces++;

        // TODO: Bug: the material base color has no effect in software renderer
        minity::color faceColor = model.material.color; // fallback if no texture

        // SEE: spaceType enum for indices
        // 0 = world, 1 = view, 2 = clip/projected space
        vec3 vects[5][3] = {};
        vec3 norms[5][3] = {};
        vec2 texc[3] = {}; // model u, v for each (model) vertice

        // VERTEX PROCESSING (model to clip space)
        for (int idx : {0, 1, 2})
        {
            processVertices(model, faceIndex, idx, vects, norms, texc,
                modelMatrix, viewMatrix, projectionMatrix,
                inverseModelMatrix, inverseViewMatrix, inverseProjectionMatrix,
                stats);
        } // end of vertex shader (space transformations)

        faceIndex++;


        if (! clippingFunction(vects[ndcCoordinates], stats))
        {
            continue; // ditch this face
        };

        if (! cullingFunction(vects[viewSpace], stats))
        {
            continue; // ditch this face
        };

        // super-simple global Illumination
        vec3 lightDirection = v3Normalize(light.translation);

        if (! hasNormals)
        {
            // FLAT SHADING
            // SEE: https://computergraphics.stackexchange.com/questions/4031/programmatically-generating-vertex-normals
            vec3 faceNormal = v3Normalize(v3CrossProduct(
                v3Sub(vects[viewSpace][2], vects[viewSpace][0]),
                v3Sub(vects[viewSpace][1], vects[viewSpace][0])));

            float dp = std::max(0.1f, v3DotProduct(lightDirection, faceNormal));
            faceColor = minity::adjustColor(faceColor, dp);
        }

        // RASTERIZATION, working in screen space

        // FRAGMENT SHADER (or pixel shader)
        // we capture the model and texture from this scope and
        // pass the shader as a lambda to the renderer
        auto fragmentShader = [&](float &u, float &v, float &w, minity::color color)
        {
            auto adjustedColor = color; // material color if no texture


            if (hasTexture && hasTextureCoordinates)
            {
                // get u, v and the corresponding pixel

                // https://stackoverflow.com/questions/74542222/whats-the-relationship-between-the-barycentric-coordinates-of-triangle-in-clip
                // and
                // https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                // Other resources:
                // https://en.wikipedia.org/wiki/Texture_mapping#Perspective_correctness
                // https://medium.com/@aminere/software-rendering-from-scratch-f60127a7cd58
                // "Perspective correct interpolation"

                assert(u >= 0 && v >= 0 && w >= 0);
                assert(1.0f - (u + v + w) < 1.0e-4f);

                // model u,v texture coordinates [0,1]
                vec2 tc1 = texc[0];
                vec2 tc2 = texc[1];
                vec2 tc3 = texc[2];

                /*
                from: https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                The formula that you will find in the GL specification (look on page 427;
                the link is the current 4.4 spec, but it has always been that way) for
                perspective-corrected interpolation of the attribute value in a triangle is:

                a * f_a / w_a   +   b * f_b / w_b   +  c * f_c / w_c
                f=-----------------------------------------------------
                    a / w_a      +      b / w_b      +     c / w_c

                where a,b,c denote the barycentric coordinates of the point in the triangle
                we are interpolating for (a,b,c >=0, a+b+c = 1), f_i the attribute value at
                vertex i, and w_i the clip space w coordinate of vertex i. Note that the
                barycentric coordinates are calculated only for the 2D projection of the
                window space coords of the triangle (so z is ignored).
                */

                float denominator = u / vects[clipSpace][0].w   +   v / vects[clipSpace][1].w  +   w / vects[clipSpace][2].w;
                float uu = ( u * tc1.u / vects[clipSpace][0].w   +   v * tc2.u / vects[clipSpace][1].w  +   w * tc3.u / vects[clipSpace][2].w ) / denominator;
                float vv = ( u * tc1.v / vects[clipSpace][0].w   +   v * tc2.v / vects[clipSpace][1].w  +   w * tc3.v / vects[clipSpace][2].w ) / denominator;

                adjustedColor = model.material.texture.get(uu, vv);
            }

            if(hasNormals)
            {
                auto n1 = norms[viewSpace][0];
                auto n2 = norms[viewSpace][1];
                auto n3 = norms[viewSpace][2];
                vec3 vn =  v3Normalize(v3Add(v3Add(v3Mul(n1, u), v3Mul(n2, v)), v3Mul(n3, w)));
                float dp = std::max(0.1f, v3DotProduct(lightDirection, vn));
                adjustedColor = minity::adjustColor(adjustedColor, dp);
            }

            return adjustedColor;
        };

        if (g_config->fillTriangles)
        {
            rasterizer.drawTriangle(vects[screenSpace], faceColor, fragmentShader);
        }
        if (g_config->drawWireframe)
        {
            for (int i = 0; i < 3; ++i)
            {
                rasterizer.drawLine(vects[screenSpace][i], vects[screenSpace][(i+1) % 3], minity::gray50);
            }
        }
        if (g_config->drawPointCloud)
        {
            for (int i = 0; i < 3; ++i)
            {
                rasterizer.drawPoint(vects[screenSpace][i], minity::white);
            }
        }
        if (g_config->drawNormals)
        {
            vec3 normal = v3Div(v3Normalize(v3CrossProduct(
                v3Sub(vects[ndcCoordinates][1], vects[ndcCoordinates][0]),
                v3Sub(vects[ndcCoordinates][2], vects[ndcCoordinates][0]))),
                10.0f);

            vec3 middle = v3Div(
                v3Add(vects[ndcCoordinates][2], v3Add(vects[ndcCoordinates][1], vects[ndcCoordinates][0])),
                3);

            vec3 tip = v3Add(middle, normal);

            middle = toScreenXY(middle, rasterizer.getViewportWidth(), rasterizer.getViewportHeight());
            tip = toScreenXY(tip, rasterizer.getViewportWidth(), rasterizer.getViewportHeight());

            rasterizer.drawLine(middle, tip, minity::white);
        }
        stats.drawnFaces++;
    }

    if (g_config->drawAxes)
    {
        auto transformWorldToScreenSpace = [&](vec3 point) {
            // no model, only world transforms
            vec3 tmp = multiplyVec3(multiplyVec3(point, viewMatrix), projectionMatrix);
            tmp = v3Div(tmp, tmp.w); // in clip space
            tmp = toScreenXY(tmp, rasterizer.getViewportWidth(), rasterizer.getViewportHeight());
            return tmp;
        };

        vec3 origin = transformWorldToScreenSpace(vec3{0.0f, 0.0f, 0.0f});
        vec3 oneX = transformWorldToScreenSpace(vec3{1.0f, 0.0f, 0.0f});
        vec3 oneY = transformWorldToScreenSpace(vec3{0.0f, 1.0f, 0.0f});
        vec3 oneZ = transformWorldToScreenSpace(vec3{0.0f, 0.0f, 1.0f});
        rasterizer.drawLine(origin, oneX, minity::red);
        rasterizer.drawLine(origin, oneY, minity::green);
        rasterizer.drawLine(origin, oneZ, minity::blue);
    }

    // show the drawn buffer
    SDLSwapBuffers(rasterizer);

    std::ostringstream stream;
    stream << stats
        << rasterizer.stats;
    g_stats = stream.str();

    return true;
}

void softwareEngine::run(scene scene)
{
    const unsigned int width{640};
    const unsigned int height{480};
    minity::rasterizer rasterizer(width, height);

    float deltaTime = 0.0f;
    auto ft = new minity::frameTimer();

    auto scale = scene.model.scale;
    auto position = scene.model.position;
    auto rotation = scene.model.rotation;

    while(m_input.handleInput())
    {
        SDL_Delay(20); // some computation budget...

        if (m_input.isKeyPressed(KEY_f))
        {
            g_config->fillTriangles = !g_config->fillTriangles;
        }
        if (m_input.isKeyPressed(KEY_l))
        {
            g_config->drawWireframe = !g_config->drawWireframe;
        }
        if (m_input.isKeyPressed(KEY_n))
        {
            g_config->drawNormals = !g_config->drawNormals;
        }
        if (m_input.isKeyPressed(KEY_p))
        {
            g_config->drawPointCloud = !g_config->drawPointCloud;
        }
        if (m_input.isKeyPressed(KEY_x))
        {
            g_config->drawAxes = !g_config->drawAxes;
        }
        if (m_input.isKeyPressed(KEY_F1))
        {
            g_config->showStatsWindow = !g_config->showStatsWindow;
        }

        scene.model.update(deltaTime);

        position = scene.model.position;
        scale = scene.model.scale;
        rotation = scene.model.rotation;

        // from: bool ok = render(scene, rasterizer);
        // to:
        //     auto metalRenderer = Renderer(layer, scene.model.mesh, scene.model.material.texture);
        //     metalRenderer.renderModel(position, scale, rotation, color);
        bool ok = render(scene, rasterizer);
        if(!ok)
        {
            std::cerr << "rendering failed - exiting" << std::endl;
            break;
        }

        SDLFPSUpdate(ft->delta());
        deltaTime = ft->deltaTime();
    }

};

void softwareEngine::shutdown()
{
    SDLEnd();
}
} // minity
