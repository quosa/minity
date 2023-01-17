#pragma once

#include <simd/simd.h> // vector_floatN

struct VertexData
{
    simd::float3 position;
    simd::float3 normal; // new for lighting
    simd::float2 texcoord; // new for texture
};

struct InstanceData
{
    simd::float4x4 instanceTransform;
    simd::float3x3 instanceNormalTransform; // new for lighting
    simd::float4 instanceColor;
};

struct CameraData
{
    simd::float4x4 perspectiveTransform;
    simd::float4x4 worldTransform;
    simd::float3x3 worldNormalTransform; // new for lighting
};
