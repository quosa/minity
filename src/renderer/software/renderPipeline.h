#include <algorithm>
#include <cassert>

#include "simpleMath.h"
#include "old_scene.h"
#include "sdlHelpers.h" // full math
#include "modelImporter.h"
// #include "lineDraw.h"
// #include "triangleFill.h" // old triangle fill algorithm
#include "frameTimer.h"
#include "rasterizer.h"
#include "stats.h"

namespace minity
{

inline vec3 &toScreenXY(vec3 &point, unsigned int screenWidth, unsigned int screenHeight)
{
    point.x = (point.x + 1.0f) * static_cast<float>(screenWidth) / 2.0f;
    point.y = (1.0f - ((point.y + 1.0f) / 2.0f)) * static_cast<float>(screenHeight);
    return point;
}

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
    screenSpace = 3,
    ndcCoordinates = 4
};

// TODO: make matrices come from model getters
// TODO: figure out if we need to add some encapsulation for vects/norms/texc
void processVertices(
    minity::model &model,
    const int idx, vec3 (&vects)[5][3], vec3 (&norms)[5][3], vec2 (&texc)[3],
    const mat4 &worldTransformations, const mat4 &viewMatrix, const mat4 &projector,
    const mat4 &inverseWorldTransformations, const mat4 &inverseViewMatrix, const mat4 &inverseProjector,
    renderStats &stats)
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
        norms[worldSpace][idx % 3] = multiplyVec3(mn, transposeMat4(inverseWorldTransformations));
    }
    if (model.hasTextureCoordinates)
    {
        texc[idx % 3] = model.textureCoordinates[idx];
    }

    // Here triangles are in WORLD SPACE
    // i.e. common coordinates for all models in scene
    // only camera, no projection

    vects[viewSpace][idx % 3] = multiplyVec3(vects[worldSpace][idx % 3], viewMatrix);
    if (model.hasNormals)
    {
        // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
        norms[viewSpace][idx % 3] = multiplyVec3(norms[worldSpace][idx % 3], transposeMat4(inverseViewMatrix));
    }

    // Here triangles are in VIEW SPACE
    // i.e. coordinates looking from camera
    // so a point at world space camera coordinates is (0,0,0)

    vects[clipSpace][idx % 3] = multiplyVec3(vects[viewSpace][idx % 3], projector);
    if (model.hasNormals)
    {
        // https://gamedev.stackexchange.com/questions/68387/how-to-modify-normal-vectors-with-a-tranformation-matrix
        norms[clipSpace][idx % 3] = multiplyVec3(norms[viewSpace][idx % 3], transposeMat4(inverseProjector));
    }

    // Here triangles are in CLIP SPACE in homogeneous coordinates
    // https://en.wikipedia.org/wiki/Homogeneous_coordinates

    // normalise into cartesian space
    vec3 vClip = vects[clipSpace][idx % 3];
    vects[ndcCoordinates][idx % 3] = v3Div(vClip, vClip.w);
    if (model.hasNormals)
    {
        vec3 nClip = norms[clipSpace][idx % 3]; // ???
        norms[ndcCoordinates][idx % 3] = v3Div(nClip, nClip.w); // ???
    }

    // Here triangles are in NDC SPACE x, y, z in [-1,1]

    // move all vertices to screen space for rasterization
    // Assumes -1 .. +1 box
    // Screen coordinates: (0,0) is top-left!
    // (x=-1, y=1) -> (0, 0)
    // (x=-1, y=-1) -> (0, sHeight)
    // (x=1, y=1) -> (sWidth, 0)
    // (x=1, y=-1) -> (sWidth, sHeight)
    // z=-1 -> 0 and z=1 -> 1

    // TODO: FIGURE OUT WHY THIS VIEW MATRIX WAS HERE???
    // vec3 v = multiplyVec3(vects[ndcCoordinates][idx % 3], viewMatrix);
    // TODO: THIS NOW LEAVES A GAP TO CENTER???
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

// TODO: add new faces if clipped
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


bool render(minity::scene scene, minity::rasterizer &rasterizer)
{
    renderStats stats{};

    minity::model &model = scene.model;
    minity::camera &camera = scene.camera;
    minity::light &light = scene.light;

    if (g_config->renderOnChange)
    {
        std::cout << "rendering a model with " << model.numFaces << " faces, ";
        std::cout << (model.hasNormals ? "" : "no ") << "normals and ";
        std::cout << (model.hasTextureCoordinates ? "" : "no ") << "texture coordinates";
        std::cout << "." << std::endl;
    }

    rasterizer.clearBuffers();

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
    mat4 inverseWorldTransformations = invertMat4(worldTransformations);

    // perspective
    float aspectRatio = (float)g_SDLWidth / (float)g_SDLHeight;
    // TODO: near and far field to camera structure
    mat4 projector = projectionMatrix(camera.fovDegrees, aspectRatio, 0.1f, 400.0f);
    mat4 inverseProjector = invertMat4(projector);

    mat4 viewMatrix = camera.getCameraMatrix();
    mat4 inverseViewMatrix = invertMat4(viewMatrix);

    mat4 lightMatrix = light.getLightTranslationMatrix();
    (void)lightMatrix; // TODO: use the light for diffusion

    for (auto face : model.faces)
    {
        stats.faces++;

        // TODO: move color to model and use minity::yellow
        u_int32_t faceColor{0xffff00ff}; // simple fallback to yellow if no texture

        // SEE: spaceType enum for indices
        // 0 = world, 1 = view, 2 = clip/projected space
        vec3 vects[5][3] = {};
        vec3 norms[5][3] = {};
        vec2 texc[3] = {}; // model u, v for each (model) vertice

        // TODO: consider changing this to work on all vertices of a face
        // instead of individual vertices. Enables us to perform
        //  back-face culling earlier
        //
        // VERTEX PROCESSING (model to clip space)
        for (int idx : face)
        {
            processVertices(model, idx, vects, norms, texc,
                worldTransformations, viewMatrix, projector,
                inverseWorldTransformations, inverseViewMatrix, inverseProjector,
                stats);
        } // end of vertex shader (space transformations)


        // vec3 v1 = vects[screenSpace][0];
        // vec3 v2 = vects[screenSpace][1];
        // vec3 v3 = vects[screenSpace][2];
        vec3 n1{};
        vec3 n2{};
        vec3 n3{};
        if (model.hasNormals)
        {
            n1 = norms[viewSpace][0];
            n2 = norms[viewSpace][1];
            n3 = norms[viewSpace][2];
        }

        if (! clippingFunction(vects[ndcCoordinates], stats)) {
            continue; // ditch this face
        };

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



        // // Print rendering debug info from each face at each stage
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
        // with viewSpace, I get the flat triangles as expected.

        // back-face culling - polygons that face away from the camera can be culled
        // we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for possible camera rotation

        auto vCameraRay = v3Normalize(v3Sub(vec3{0}, vects[viewSpace][0])); // in view space camera is at origin, so pick one vertice and calculate direction towards it
        auto faceDotCamera = v3DotProduct(faceNormal, vCameraRay);

        if (faceDotCamera <= 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal << " camera:" << vCameraRay << " dot: " << std::to_string(faceDotCamera) << std::endl << std::endl;
            // std::cout << "            face[0]: " << vects[viewSpace][0] << " [1]: " << vects[viewSpace][1] << " [2]: " << vects[viewSpace][2] << std::endl;
            stats.bfCulled++;
            continue;
        }

        // super-simple global Illumination
        vec3 lightDirection = v3Normalize(light.translation);

        if (!model.hasNormals)
        {
            // FLAT SHADING
            float dp = std::max(0.1f, v3DotProduct(lightDirection, faceNormal));
            faceColor = adjustColor(faceColor, dp);
            // std::cout << "face normal " << faceNormal << " dp is " << dp << std::endl;
            // printColor(faceColor);
        }

        // RASTERIZATION, working in screen space


        // FRAGMENT SHADER (or pixel shader)
        // we capture the model and texture from this scope and
        // pass the shader as a lambda to the renderer
        auto fragmentShader = [&](float &u, float &v, float &w, minity::color color)
        {
            // std::cout << "fragment shader with barycentric coordinates: " << u << ", " << v << ", " << w << std::endl;
            u_int32_t adjustedColor = color;
            // calculate normal at (x, y) and adjust face color
            // Phong shading?

            if (model.hasTextureCoordinates && model.hasTexture)
            {
                // get u, v and the corresponding pixel

                // https://stackoverflow.com/questions/74542222/whats-the-relationship-between-the-barycentric-coordinates-of-triangle-in-clip
                // and
                // https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                // Other resources:
                // https://en.wikipedia.org/wiki/Texture_mapping#Perspective_correctness
                // https://medium.com/@aminere/software-rendering-from-scratch-f60127a7cd58
                // "Perspective correct interpolation"


                // std::cout << "screen space calculated  barycentric coordinates: " << u << ", " << v << ", " << w << std::endl;
                // assert(u >= 0 && v >= 0 && w >= 0 && u + v + w == 1);
                assert(u >= 0 && v >= 0 && w >= 0);
                assert(1.0f - (u + v + w) < 1.0e-4f);

                // model u,v texture coordinates [0,1]
                vec2 tc1 = texc[0];
                vec2 tc2 = texc[1];
                vec2 tc3 = texc[2];

                // trivial affine texture maping (produces a dent in the texture)
                // float uu = tc1.u * u + tc2.u * v + tc3.u * w;
                // float vv = tc1.v * u + tc2.v * v + tc3.v * w;

                // std::cout << "model texture coordinates: " << tc1 << ", " << tc2 << ", " << tc3 << std::endl;

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
                // std::cout << "texture coordinates: " << uu << ", " << vv << std::endl;

                adjustedColor = model.texture->get(uu, vv);
                // std::cout << "texture coordinates: " << uu << ", " << vv << " color: ";
                // printColor(adjustedColor);
                // std::cout << std::endl;
            }

            if (model.hasNormals)
            {
                vec3 vn =  v3Normalize(v3Add(v3Add(v3Mul(n1, u), v3Mul(n2, v)), v3Mul(n3, w)));
                float dp = std::max(0.1f, v3DotProduct(lightDirection, vn));
                adjustedColor = adjustColor(adjustedColor, dp);
                // std::cout << "barycentric normal " << vn << " dp is " << dp << std::endl;
                // printColor(adjustedColor);
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
            vec3 tmp = multiplyVec3(multiplyVec3(point, viewMatrix), projector);
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

    if (g_config->renderOnChange)
    {
        std::cout << stats << std::endl;
        std::cout << rasterizer.stats << std::endl;
    }

    std::ostringstream stream;
    stream << stats
        << rasterizer.stats;
    g_stats = stream.str();

    return true;
}


void run(minity::scene scene)
{
    const unsigned int width{640};
    const unsigned int height{480};
    minity::rasterizer rasterizer(width, height);

    float deltaTime = 0.0f;
    auto ft = new minity::frameTimer();

    vec3 inputTranslation{};
    vec3 inputRotation{};
    vec3 zeroVector{};

    bool inputChange{false};
    while (isRunning(&inputTranslation, &inputRotation, &inputChange))
    {
        SDL_Delay(20); // some computation budget...

        if (!(g_config->renderOnChange && !inputChange))
        {
            bool translateChange = inputTranslation != zeroVector || inputRotation != zeroVector;
            if (translateChange)
            {
                scene.camera.translation = v3Add(scene.camera.translation, inputTranslation);
                scene.camera.rotation = v3Add(scene.camera.rotation, inputRotation);
                std::cout << "cam: position " << scene.camera.translation.str()
                        << " rotation " << scene.camera.rotation.str() << std::flush << std::endl;
            }

            scene.model.rotation.y += deg2rad(deltaTime * 10.0f);

            bool ok = render(scene, rasterizer);
            if(!ok)
            {
                std::cerr << "rendering failed - exiting" << std::endl;
                break;
            }
        }
        SDLFPSUpdate(ft->delta());
        deltaTime = ft->deltaTime();
        inputTranslation = vec3{};
        inputRotation = vec3{};
    }
}

void shutdown()
{
    SDLEnd();
}
} // minity