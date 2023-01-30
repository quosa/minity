#pragma once

#include "shader_types.h"

struct Scene
{
    VertexData *vertexData;
    size_t vertexDataSize;
    u_int32_t *indexData;
    size_t indexDataSize;
};



// simple triangle face with normals and texture coordinates
// TODO: return a mesh instead of scene
Scene *GetSingleFaceScene()
{
    VertexData *simpleFaceVertexData = new VertexData[3]{
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -1.0f, -1.0f, 0.0f }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { -1.0f, 1.0f, 0.0f }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
        { { 1.0f, 1.0f, 0.0f }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } }
    };
    uint32_t *simpleFaceIndexData = new uint32_t[3]{0, 1, 2}; // CW/LHW
    const size_t simpleFaceVertexDataSize = 3 * sizeof(VertexData);
    const size_t simpleFaceIndexDataSize = 3 * sizeof(uint32_t);

    Scene *simpleFaceScene = new Scene{simpleFaceVertexData, simpleFaceVertexDataSize, simpleFaceIndexData, simpleFaceIndexDataSize};
    return simpleFaceScene;
}

// simple cube with normals and texture coordinates
// TODO: return a mesh instead of scene
Scene *GetCubeScene()
{
    const float s = 0.5f;

    VertexData *vertexData = new VertexData[24]{
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -s, -s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { +s, -s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
        { { +s, +s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
        { { -s, +s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },

        { { +s, -s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { +s, -s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 1.f } },
        { { +s, +s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { +s, +s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 0.f } },

        { { +s, -s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },
        { { -s, -s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },
        { { -s, +s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
        { { +s, +s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },

        { { -s, -s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { -s, -s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 1.f } },
        { { -s, +s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { -s, +s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 0.f } },

        { { -s, +s, +s }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },
        { { +s, +s, +s }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
        { { +s, +s, -s }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
        { { -s, +s, -s }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },

        { { -s, -s, -s }, {  0.f, -1.f,  0.f }, { 0.f, 1.f } },
        { { +s, -s, -s }, {  0.f, -1.f,  0.f }, { 1.f, 1.f } },
        { { +s, -s, +s }, {  0.f, -1.f,  0.f }, { 1.f, 0.f } },
        { { -s, -s, +s }, {  0.f, -1.f,  0.f }, { 0.f, 0.f } }
    };

    // faces in counter-clockwise (right-hand rule) winding order
    uint32_t *indexData = new uint32_t[36]{
        0,  1,  2,  2,  3,  0, /* front */
        4,  5,  6,  6,  7,  4, /* right */
        8,  9, 10, 10, 11,  8, /* back */
        12, 13, 14, 14, 15, 12, /* left */
        16, 17, 18, 18, 19, 16, /* top */
        20, 21, 22, 22, 23, 20, /* bottom */
    };

    const size_t vertexDataSize = 24 * sizeof( VertexData );
    const size_t indexDataSize = 36 * sizeof( uint32_t );

    Scene *scene = new Scene{vertexData, vertexDataSize, indexData, indexDataSize};
    return scene;
}
