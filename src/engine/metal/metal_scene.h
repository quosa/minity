#pragma once

#include "shader_types.h"

struct Scene
{
    VertexData *vertexData;
    size_t vertexDataSize;
    u_int32_t *indexData;
    size_t indexDataSize;
};
