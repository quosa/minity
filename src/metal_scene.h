#pragma once

#include "shader_types.h"

struct Scene
{
    VertexData *vertexData;
    size_t vertexDataSize;
    uint16_t *indexData;
    size_t indexDataSize;
};
