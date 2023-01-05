#include <algorithm>
#include "simpleMath.h"
#include "scene.h"
#include "sdlHelpers.h" // full math
#include "modelImporter.h"
#include "lineDraw.h"
#include "triangleFill.h" // old triangle fill algorithm
#include "frameTimer.h"

namespace minity
{

const u_int32_t blue = 0x0000ffff;
const u_int32_t yellow = 0xffff00ff;

// Barycentric:
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html (2d)

/* THIS IS JUST FOR REFERENCE:
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
struct barycentricCoordinates
{
public:
    // pre-calculate everything that we can before
    // for x, y hot-path loop
    void prepare(const vec3 face[3])
    {
        v0 = v3Sub(face[1], face[0]); // b - a
        v1 = v3Sub(face[2], face[0]); // c - a;
        d00 = v3DotProduct(v0, v0);
        d01 = v3DotProduct(v0, v1);
        d11 = v3DotProduct(v1, v1);
        invDenom = 1.0f / (d00 * d11 - d01 * d01);
    }

    // calculate only the missing elements for each x, y
    // and return the corresponding barycentric coordinates
    // TODO: change out parameters to return a vec3 with the coordinates
    //       will be used as barycentric.x etc.
    void barycentricCoordinatesAt(const vec3 face[3], const vec3 &point, float &u, float &v, float &w)
    {
        assert(invDenom != 0.0f);
        v2 = v3Sub(point, face[0]); // p - a;
        d20 = v3DotProduct(v2, v0);
        d21 = v3DotProduct(v2, v1);
        v = (d11 * d20 - d01 * d21) * invDenom;
        w = (d00 * d21 - d01 * d20) * invDenom;
        u = 1.0f - v - w;
    }

private:
    vec3 v0{0};
    vec3 v1{0};
    vec3 v2{0};
    float d00{0};
    float d01{0};
    float d11{0};
    float d20{0};
    float d21{0};
    float invDenom;
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
    screenSpace = 3,
    ndcCoordinates = 4
};

struct renderStats
{
    unsigned long int vertices{0};
    unsigned long int faces{0};
    unsigned long int bfCulled{0};
    unsigned long int vfCulled{0};
    unsigned long int xyClipped{0};
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
       << "  bf-culled    " << stats.bfCulled << std::endl
       << "  vf-culled    " << stats.vfCulled << std::endl
       << "  drawn faces  " << stats.drawnFaces << std::endl
       << "  xy-clipped   " << stats.xyClipped << std::endl
       << "  outside      " << stats.outside << std::endl
       << "  inside       " << stats.inside << std::endl
       << "  in : out     " << pixelPercentage << std::endl
       << "  drawn points " << stats.drawnPoints << std::endl
       << "  depth        " << stats.depth << std::endl
       << "  buffer size  " << stats.bufferSize << std::endl;
    return os;
}

bool render(minity::scene scene)
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

    SDLClearBuffers();

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

    mat4 viewMatrix = camera.getCameraMatrix();

    mat4 lightMatrix = light.getLightTranslationMatrix();
    (void)lightMatrix; // TODO: use the light for diffusion

    for (auto face : model.faces)
    {
        stats.faces++;

        // TODO: move color to model
        u_int32_t faceColor = yellow; // simple fallback if no texture

        // SEE: spaceType enum for indices
        // 0 = world, 1 = view, 2 = clip/projected space
        vec3 vects[5][3] = {};
        vec3 norms[5][3] = {};
        vec2 texc[3] = {}; // model u, v for each (model) vertice

        // TODO: consider changing this to work on all vertices of a face
        // instead of individual vertices. Enables us to perform
        //  back-face culling earlier
        //
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
            vec3 v = multiplyVec3(vects[ndcCoordinates][idx % 3], viewMatrix);
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
        vec3 v1 = vects[screenSpace][0];
        vec3 v2 = vects[screenSpace][1];
        vec3 v3 = vects[screenSpace][2];
        vec3 n1{};
        vec3 n2{};
        vec3 n3{};
        if (model.hasNormals)
        {
            n1 = norms[viewSpace][0];
            n2 = norms[viewSpace][1];
            n3 = norms[viewSpace][2];
        }

        // view volume culling (outside frustrum)
        int numFacesOutside = 0;
        for (auto &v : vects[ndcCoordinates])
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
            // std::cout << "discarding face - view volume culling 3 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
            stats.vfCulled++;
            continue;
        case 2:
            // TODO: bring the 2 vertices that are outside to screen border
            // std::cout << "discarding face - view volume culling 2 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
            stats.vfCulled++;
            continue;
        case 1:
            // TODO: split the base into 2 triangles and change the new vertices to screen border
            // std::cout << "discarding face - view volume culling 1 outside " << vects[ndcCoordinates][0].str() << " " << vects[ndcCoordinates][1].str() << " " << vects[ndcCoordinates][2].str() << std::endl;
            stats.vfCulled++;
            continue;
        case 0:
            // face fully within frustrum - render normally
            break;
        default:
            std::cerr << "unknown number of faces putside camera frustrum " << numFacesOutside << " - exiting" << std::endl;
            exit(1);
            break;
        }

        assert(numFacesOutside <= 1);

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
        // with viewSpace, I get the flat triangles as expected.

        // back-face culling - polygons that face away from the camera can be culled
        // we do it in view space (=camera coordinates)
        // cannot do it in world space as we have to anyway adjust for possible camera rotation

        auto vCameraRay = v3Normalize(v3Sub(vec3{0}, vects[viewSpace][0])); // in view space camera is at origin, so pick one vertice and calculate direction towards it
        auto faceDotCamera = v3DotProduct(faceNormal, vCameraRay);

        if (faceDotCamera <= 0.0f)
        {
            // std::cout << "CULLING face normal: " << faceNormal << " camera:" << vCameraRay << " dot: " << std::to_string(faceDotCamera) << std::endl << std::endl;
            // std::cout << "            face[0]: " << view.vertices[0].str() << " [1]: " << view.vertices[1].str() << " [2]: " << view.vertices[2].str() << std::endl;
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

        if (g_config->fillTriangles)
        {


            /* get the bounding box of the triangle */
            int maxX = std::max(v1.x, std::max(v2.x, v3.x));
            int minX = std::min(v1.x, std::min(v2.x, v3.x));
            int maxY = std::max(v1.y, std::max(v2.y, v3.y));
            int minY = std::min(v1.y, std::min(v2.y, v3.y));

            // std::cout << "v1:" << v1.str() << " v2:" << v2.str() << " v3:" << v3.str() <<std::endl;
            // std::cout << "BB:("<<minX<<","<<minY<<") ("<<maxX<<","<<maxY<<")"<<std::endl;

            // barycentric coordinates
            float u{0};
            float v{0};
            float w{0};
            barycentricCoordinates bc{};

            // minor optimization: calculate
            // the constant parts only once
            bc.prepare(vects[screenSpace]);

            for (int x = minX; x <= maxX; x++)
            {
                for (int y = minY; y <= maxY; y++)
                {

                    // TODO: many implementations move the inspection point to center of pixel
                    //       need to compare the output and see if this helps with tears etc.
                    // vec3 point  = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, 0};
                    vec3 point  = {static_cast<float>(x), static_cast<float>(y), 0}; // IS THIS 0 CORRECT ???
                    bc.barycentricCoordinatesAt(vects[screenSpace], point, u, v, w);

                    // barycentricCoordinatesAt(vects[screenSpace], point, u, v, w);

                    if (u < 0 || v < 0 || w < 0)
                    {
                        stats.outside++;
                        continue; // we're outside the triangle
                    }
                    stats.inside++;


                    // clipping #3 to viewport/projection space
                    if (x < 0 || x >= g_SDLWidth || y < 0 || y >= g_SDLHeight)
                    {
                        stats.xyClipped++;
                        continue; // we're outside the viewport
                    }

                    // z-buffer (depth) check
                    // get the z value for this point using the barymetric coordinates:
                    float z = v1.z * u + v2.z * v + v3.z * w;

                    // we need to use 1/z because the depth cannot be interpolated linearly!
                    // see: https://gabrielgambetta.com/computer-graphics-from-scratch/12-hidden-surface-removal.html#why-1z-instead-of-z
                    // and https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation.html
                    // float inv_z = u / v1.z + v / v2.z + w / v3.z;

                    // std::cout << "Z-buffer check: inv_z (bary) " << inv_z << " z-buffer(x,y) " << g_DepthBuffer[y * g_SDLWidth + x] << std::endl;


                    // std::cout << x << " " << y << " " << z << " vs (" << g_DepthBuffer[y * g_SDLWidth + x] << ")" << std::endl;
                    // TODO: < or <= here? Do we draw the new pixel if x, y, z are the same?
                    // camera looks at Z- so
                    // furthest away 1/z is 1/-inf = -0 and
                    // close 1/z is e.g. 1/-10 = -0.1
                    // so we need to compare with <=
                    // if (inv_z <= g_DepthBuffer[y * g_SDLWidth + x])

                    // std::cout << "z: " << z << " <= " << g_DepthBuffer[y * g_SDLWidth + x] << "?" << std::endl;

                    if (z <= g_DepthBuffer[y * g_SDLWidth + x])
                    {

                        u_int32_t c = faceColor;
                        // calculate normal at (x, y) and adjust face color
                        // Phong shading?

                        if (model.hasTextureCoordinates && model.hasTexture)
                        {
                            // get u, v and the corresponding pixel

    #if 0 // texture perspective correction
                            // https://en.wikipedia.org/wiki/Texture_mapping#Perspective_correctness
                            // https://medium.com/@aminere/software-rendering-from-scratch-f60127a7cd58
                            // "Perspective correct interpolation"
                            vec3 tc1 = {texc[0].u, texc[0].v, 1};
                            vec3 tc2 = {texc[1].u, texc[1].v, 1};
                            vec3 tc3 = {texc[2].u, texc[2].v, 1};
                            std::cout << "model texture coordinates: " << tc1 << ", " << tc2 << ", " << tc3 << std::endl;
                            tc1 = v3Div(tc1, vects[clipSpace][0].w);
                            tc2 = v3Div(tc1, vects[clipSpace][1].w);
                            tc3 = v3Div(tc1, vects[clipSpace][2].w);
                            std::cout << "texture coordinates / clip.w: " << tc1 << ", " << tc2 << ", " << tc3 << std::endl;

                            // float wt = coords.x * at.z + coords.y * bt.z + coords.z * ct.z;
                            float wt = u * tc1.z + v * tc2.z + w * tc3.z;
                            std::cout << "wt " << wt << std::endl;

                            float uu = (tc1.x * u + tc2.x * v + tc3.x * w) / wt;
                            float vv = (tc1.y * u + tc2.y * v + tc3.y * w) / wt;
                            std::cout << "texture coordinates: " << uu << ", " << vv << std::endl;
    #endif // texture perspective correction

    #if 0 // affine texture mapping
                            // https://en.wikipedia.org/wiki/Texture_mapping#Affine_texture_mapping
                            // working (u,v) coordinates that have the perspective dent

                            vec2 tc1 = texc[0];
                            vec2 tc2 = texc[1];
                            vec2 tc3 = texc[2];
                            float uu = tc1.u * u + tc2.u * v + tc3.u * w;
                            float vv = tc1.v * u + tc2.v * v + tc3.v * w;

                            // std::cout << "v1 " << v1 << " v2 " << v2 << " v3 " << v3 << std::endl;
                            // std::cout << "tc1 " << tc1 << " tc2 " << tc2 << " tc3 " << tc3 << std::endl;
    #endif // affine

    #if 1 // clip-space barycentric coordinates
                            // https://stackoverflow.com/questions/74542222/whats-the-relationship-between-the-barycentric-coordinates-of-triangle-in-clip
                            // and
                            // https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation

                            // std::cout << "screen space calculated  barycentric coordinates: " << u << ", " << v << ", " << w << std::endl;
                            // assert(u >= 0 && v >= 0 && w >= 0 && u + v + w == 1);
                            assert(u >= 0 && v >= 0 && w >= 0);
                            assert(1.0f - (u + v + w) < 1.0e-4f);

                            // model u,v texture coordinates [0,1]
                            vec2 tc1 = texc[0];
                            vec2 tc2 = texc[1];
                            vec2 tc3 = texc[2];

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

    #endif // clip-space barycentric coordinates

                            c = model.texture->get(uu, vv);
                            // std::cout << "texture coordinates: " << uu << ", " << vv << " color: ";
                            // printColor(c);
                            // std::cout << std::endl;
                        }

                        if (model.hasNormals)
                        {
                            vec3 vn =  v3Normalize(v3Add(v3Add(v3Mul(n1, u), v3Mul(n2, v)), v3Mul(n3, w)));
                            float dp = std::max(0.1f, v3DotProduct(lightDirection, vn));
                            c = adjustColor(c, dp);
                            // std::cout << "barycentric normal " << vn << " dp is " << dp << std::endl;
                            // printColor(c);
                        }

                        // std::cout << "drawing to " << p << std::endl;
                        g_SDLBackBuffer[y * g_SDLWidth + x] = c;
                        g_DepthBuffer[y * g_SDLWidth + x] = z;
                        // g_DepthBuffer[y * g_SDLWidth + x] = inv_z;
                        stats.drawnPoints++;
                    }
                    else
                    {
                        stats.depth++;
                    }
                }
            }


        } // fillTriangles

        if (g_config->drawWireframe)
        {
            vec3 cv1 = vects[ndcCoordinates][0];
            vec3 cv2 = vects[ndcCoordinates][1];
            vec3 cv3 = vects[ndcCoordinates][2];

            // cheat the z-buffer to actually draw the line
            // RH coordinates, so add just a bit
            v1.z -= 0.101;
            v2.z -= 0.101;
            v3.z -= 0.101;
            drawLine(cv1, cv2, (u_int32_t)0xeeeeeeff);
            drawLine(cv2, cv3, (u_int32_t)0xeeeeeeff);
            drawLine(cv3, cv1, (u_int32_t)0xeeeeeeff);
        }

        if (g_config->drawNormals)
        {
            vec3 projectedFaceNormal = v3Div(v3Normalize(v3CrossProduct(
                v3Sub(vects[ndcCoordinates][1], vects[ndcCoordinates][0]),
                v3Sub(vects[ndcCoordinates][2], vects[ndcCoordinates][0]))),
                10.0f);

            vec3 middle = v3Div(
                v3Add(vects[ndcCoordinates][2], v3Add(vects[ndcCoordinates][1], vects[ndcCoordinates][0])),
                3);

            drawLine(middle, v3Add(middle, projectedFaceNormal), 0xffffffff); // faceNormal sticking out from the face
        }

        stats.drawnFaces++;
    }

    // show the drawn buffer
    SDLSwapBuffers();

    if (g_config->renderOnChange)
    {
        std::cout << stats << std::endl;
    }
    std::ostringstream stream;
    stream << stats;
    g_stats = stream.str();

    return true;
}


void run(minity::scene scene)
{
    float deltaTime = 0.0f;
    auto ft = new minity::frameTimer();

    vec3 inputTranslation{};
    vec3 inputRotation{};
    vec3 zeroVector{};

    while (isRunning(&inputTranslation, &inputRotation))
    {
        SDL_Delay(20); // some computation budget...

        bool inputChange = inputTranslation != zeroVector || inputRotation != zeroVector;
        if (g_config->renderOnChange && !inputChange)
        {
            continue;
        }

        if (inputChange)
        {
            scene.camera.translation = v3Add(scene.camera.translation, inputTranslation);
            scene.camera.rotation = v3Add(scene.camera.rotation, inputRotation);
            std::cout << "cam: position " << scene.camera.translation.str()
                      << " rotation " << scene.camera.rotation.str() << std::flush << std::endl;
        }

        scene.model.rotation.y += deg2rad(deltaTime * 10.0f);

        bool ok = render(scene);
        if(!ok)
        {
            std::cerr << "rendering failed - exiting" << std::endl;
            break;
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
