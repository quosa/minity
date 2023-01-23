#pragma once

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h> // vector_uintN

#include <SDL2/SDL.h>
#include <iostream>

#include "shader_types.h"
#include "scene.h"
#include "metal_scene.h"
#include "renderer_metallib.h"

static constexpr size_t kMaxFramesInFlight = 3;


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"
// ignore the warnings from metal-cpp :-/
// src/renderer.h:49:16: error: compound literals are a C99-specific feature [-Werror,-Wc99-extensions]
//         return (simd_float4x4){ (float4){ 1.f, 0.f, 0.f, 0.f },
namespace math
{
    constexpr simd::float3 add( const simd::float3& a, const simd::float3& b );
    constexpr simd_float4x4 makeIdentity();
    simd::float4x4 makePerspective( float fovRadians, float aspect, float znear, float zfar );
    simd::float4x4 makeXRotate( float angleRadians );
    simd::float4x4 makeYRotate( float angleRadians );
    simd::float4x4 makeZRotate( float angleRadians );
    simd::float4x4 makeTranslate( const simd::float3& v );
    simd::float4x4 makeScale( const simd::float3& v );
    simd::float3x3 discardTranslation( const simd::float4x4& m );
// }


// namespace math
// {
    constexpr simd::float3 add( const simd::float3& a, const simd::float3& b )
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    constexpr simd_float4x4 makeIdentity()
    {
        using simd::float4;
        return (simd_float4x4){ (float4){ 1.f, 0.f, 0.f, 0.f },
                                (float4){ 0.f, 1.f, 0.f, 0.f },
                                (float4){ 0.f, 0.f, 1.f, 0.f },
                                (float4){ 0.f, 0.f, 0.f, 1.f } };
    }

    simd::float4x4 makePerspective( float fovRadians, float aspect, float znear, float zfar )
    {
        using simd::float4;
        float ys = 1.f / tanf(fovRadians * 0.5f);
        float xs = ys / aspect;
        float zs = zfar / ( znear - zfar );
        return simd_matrix_from_rows((float4){ xs, 0.0f, 0.0f, 0.0f },
                                     (float4){ 0.0f, ys, 0.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, zs, znear * zs },
                                     (float4){ 0, 0, -1, 0 });
    }

    simd::float4x4 makeXRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ 1.0f, 0.0f, 0.0f, 0.0f },
                                     (float4){ 0.0f, cosf( a ), sinf( a ), 0.0f },
                                     (float4){ 0.0f, -sinf( a ), cosf( a ), 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeYRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ cosf( a ), 0.0f, sinf( a ), 0.0f },
                                     (float4){ 0.0f, 1.0f, 0.0f, 0.0f },
                                     (float4){ -sinf( a ), 0.0f, cosf( a ), 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeZRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ cosf( a ), sinf( a ), 0.0f, 0.0f },
                                     (float4){ -sinf( a ), cosf( a ), 0.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, 1.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeTranslate( const simd::float3& v )
    {
        using simd::float4;
        const float4 col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
        const float4 col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
        const float4 col2 = { 0.0f, 0.0f, 1.0f, 0.0f };
        const float4 col3 = { v.x, v.y, v.z, 1.0f };
        return simd_matrix( col0, col1, col2, col3 );
    }

    simd::float4x4 makeScale( const simd::float3& v )
    {
        using simd::float4;
        return simd_matrix((float4){ v.x, 0, 0, 0 },
                           (float4){ 0, v.y, 0, 0 },
                           (float4){ 0, 0, v.z, 0 },
                           (float4){ 0, 0, 0, 1.0 });
    }

    simd::float3x3 discardTranslation( const simd::float4x4& m )
    {
        return simd_matrix( m.columns[0].xyz, m.columns[1].xyz, m.columns[2].xyz );
    }

}
#pragma clang diagnostic pop

class Renderer
{
public:
    Renderer(CA::MetalLayer *layer, Scene &scene, minity::image& texture);
    ~Renderer();
    // one render pass
    void renderModel(const simd::float3 &position, const simd::float3 &scale, const float &angle, const simd::float4 &color);
private:
    void initPipeline();
    void initDepthTexture();
    void initDepthStencil();
    void initTexture(minity::image& texture);
    void initBuffers(Scene &scene);

    CA::MetalLayer *layer;
    MTL::Device *device{nullptr};

    dispatch_semaphore_t semaphore;
    MTL::RenderPipelineState *pipeline{nullptr};
    MTL::DepthStencilState *depth_stencil{nullptr};
    MTL::Texture *pTexture{nullptr};
    MTL::Texture *pDepthTexture{nullptr};


    MTL::Buffer* vertex_data_buffer;
    MTL::Buffer* index_buffer;
    unsigned int indexBufferCount{0};
    MTL::Buffer* instace_data_buffer[kMaxFramesInFlight];
    MTL::Buffer* camera_data_buffer[kMaxFramesInFlight];
    MTL::CommandQueue *queue{nullptr};

    int _frame; // holds which buffer to fill, see kMaxFramesInFlight

};

Renderer::Renderer(CA::MetalLayer *layer, Scene &scene, minity::image& texture) : layer(layer)
{
    device = layer->device();

    auto name = device->name();
    std::cerr << "device name: " << name->utf8String() << std::endl;

    semaphore = dispatch_semaphore_create( kMaxFramesInFlight );
    initPipeline();
    initDepthTexture();
    initDepthStencil();
    initTexture(texture);
    initBuffers(scene);
};

Renderer::~Renderer()
{
    depth_stencil->release();
    pTexture->release();
    pipeline->release();
    queue->release();
}

void Renderer::initPipeline()
{
    ///////////////////////////////////////////////////////////////////////////////////////
    // shaders
    auto library_data = dispatch_data_create(
        &renderer_metallib[0], renderer_metallib_len,
        dispatch_get_main_queue(),
        ^{ });

    NS::Error *err;
    auto library = device->newLibrary(library_data, &err);

    if (!library) {
        std::cerr << "Failed to create library" << std::endl;
        std::exit(-1);
    }

    auto vertex_function_name = NS::String::string("vertexMain", NS::ASCIIStringEncoding);
    auto vertex_function = library->newFunction(vertex_function_name);
    vertex_function_name->release();

    auto fragment_function_name = NS::String::string("fragmentMain", NS::ASCIIStringEncoding);
    auto fragment_function = library->newFunction(fragment_function_name);
    fragment_function_name->release();

    auto pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipeline_descriptor->setVertexFunction(vertex_function);
    pipeline_descriptor->setFragmentFunction(fragment_function);

    pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB );
    pipeline_descriptor->setDepthAttachmentPixelFormat( MTL::PixelFormat::PixelFormatDepth32Float_Stencil8 );
    pipeline_descriptor->setStencilAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float_Stencil8);

    vertex_function->release();
    fragment_function->release();

    auto color_attachment_descriptor = pipeline_descriptor->colorAttachments()->object(0);
    color_attachment_descriptor->setPixelFormat(layer->pixelFormat());

    pipeline = device->newRenderPipelineState(pipeline_descriptor, &err);

    pipeline_descriptor->release();

    if (!pipeline) {
        std::cerr << "Failed to create pipeline" << std::endl;
        std::exit(-1);
    }
}

// https://stackoverflow.com/questions/42637659/trouble-getting-depth-testing-to-work-with-apples-metal-graphics-api
void Renderer::initDepthTexture()
{
    auto drawableSize = layer->drawableSize();
    auto depthTextureDesc = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormat::PixelFormatDepth32Float_Stencil8, // was: PixelFormatDepth16Unorm,
        drawableSize.width,
        drawableSize.height,
        false
    );
    depthTextureDesc->setStorageMode(MTL::StorageModePrivate);
    depthTextureDesc->setUsage(MTL::TextureUsageRenderTarget);
    pDepthTexture = device->newTexture( depthTextureDesc );
}

void Renderer::initDepthStencil()
{
    ///////////////////////////////////////////////////////////////////////////////////////
    // depth stencil states
    auto depth_stencil_descriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depth_stencil_descriptor->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionLess );
    depth_stencil_descriptor->setDepthWriteEnabled( true );

    depth_stencil = device->newDepthStencilState( depth_stencil_descriptor );

    depth_stencil_descriptor->release();

    if (!depth_stencil) {
        std::cerr << "Failed to create depth stencil" << std::endl;
        std::exit(-1);
    }
}

void Renderer::initTexture(minity::image& texture)
{
    ///////////////////////////////////////////////////////////////////////////////////////
    // Textures

    const uint32_t tw = texture.width;
    const uint32_t th = texture.height;

    MTL::TextureDescriptor* pTextureDesc = MTL::TextureDescriptor::alloc()->init();
    pTextureDesc->setWidth( tw );
    pTextureDesc->setHeight( th );
    pTextureDesc->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
    pTextureDesc->setTextureType( MTL::TextureType2D );
    pTextureDesc->setStorageMode( MTL::StorageModeManaged );
    pTextureDesc->setUsage( MTL::ResourceUsageSample | MTL::ResourceUsageRead );

    pTexture = device->newTexture( pTextureDesc );

    pTexture->replaceRegion( MTL::Region( 0, 0, 0, tw, th, 1 ), 0, texture._getRawData(), texture.width * 4 );

    pTextureDesc->release();
}

void Renderer::initBuffers(Scene &scene)
{
    ///////////////////////////////////////////////////////////////////////////////////////
    // buffers

    vertex_data_buffer = device->newBuffer( scene.vertexDataSize, MTL::ResourceStorageModeManaged );
    memcpy( vertex_data_buffer->contents(), scene.vertexData, scene.vertexDataSize );
    vertex_data_buffer->didModifyRange( NS::Range::Make( 0, vertex_data_buffer->length() ) );

    index_buffer = device->newBuffer( scene.indexDataSize, MTL::ResourceStorageModeManaged );
    memcpy( index_buffer->contents(), scene.indexData, scene.indexDataSize );
    index_buffer->didModifyRange( NS::Range::Make( 0, index_buffer->length() ) );
    indexBufferCount = scene.indexDataSize / sizeof(u_int32_t);

    const size_t instanceDataSize = kMaxFramesInFlight * sizeof( InstanceData );
    for ( size_t i = 0; i < kMaxFramesInFlight; ++i )
    {
        instace_data_buffer[ i ] = device->newBuffer( instanceDataSize, MTL::ResourceStorageModeManaged );
    }

    const size_t cameraDataSize = kMaxFramesInFlight * sizeof( CameraData );
    for ( size_t i = 0; i < kMaxFramesInFlight; ++i )
    {
        camera_data_buffer[ i ] = device->newBuffer( cameraDataSize, MTL::ResourceStorageModeManaged );
    }

    queue = device->newCommandQueue();
};

void Renderer::renderModel(const simd::float3 &position, const simd::float3 &scale, const float &angle, const simd::float4 &color)
{
    auto drawable = layer->nextDrawable();
    using simd::float3;
    using simd::float4;
    using simd::float4x4;

    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    _frame = (_frame + 1) % kMaxFramesInFlight;
    MTL::Buffer* pInstanceDataBuffer = instace_data_buffer[ _frame ];

    MTL::CommandBuffer* buffer = queue->commandBuffer();
    dispatch_semaphore_wait( semaphore, DISPATCH_TIME_FOREVER );
    buffer->addCompletedHandler( ^void(  MTL::CommandBuffer* buffer ){
        (void)buffer; // silence linter
        dispatch_semaphore_signal( semaphore );
    });

    InstanceData* pInstanceData = reinterpret_cast< InstanceData *>( pInstanceDataBuffer->contents() );

    //

    // Update instance positions:
    // Use the tiny math library to apply a 3D transformation to the instance.
    float4x4 translate = math::makeTranslate( position );
    float4x4 scaler = math::makeScale( scale );
    float4x4 xrot = math::makeXRotate( 0.0f ); // angle );
    float4x4 zrot = math::makeZRotate( 0.0f ); // angle );
    float4x4 yrot = math::makeYRotate( angle );

    pInstanceData->instanceTransform = translate * xrot * yrot * zrot * scaler;
    pInstanceData->instanceNormalTransform = math::discardTranslation( pInstanceData->instanceTransform );
    pInstanceData->instanceColor = color;
    pInstanceDataBuffer->didModifyRange( NS::Range::Make( 0, sizeof( InstanceData) ) );

    // Update camera state:

    MTL::Buffer* pCameraDataBuffer = camera_data_buffer[ _frame ];
    CameraData* pCameraData = reinterpret_cast< CameraData *>( pCameraDataBuffer->contents() );
    // pCameraData->perspectiveTransform = math::makePerspective( 45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f ) ;
    auto drawableSize = layer->drawableSize();
    float aspectRatio = (float) (drawableSize.width / drawableSize.height);
    pCameraData->perspectiveTransform = math::makePerspective( 60.0f * M_PI / 180.f, aspectRatio, 0.1f, 500.0f ) ;
    pCameraData->worldTransform = math::makeIdentity();
    pCameraData->worldNormalTransform = math::discardTranslation( pCameraData->worldTransform );
    pCameraDataBuffer->didModifyRange( NS::Range::Make( 0, sizeof( CameraData ) ) );

    // Begin render pass:

    // as we're not using MTKView, we need to do this manually
    // MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
    // MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );

    auto pass = MTL::RenderPassDescriptor::renderPassDescriptor();

    // https://stackoverflow.com/questions/29495380/ios-metal-how-to-clear-depth-buffer-similar-to-glcleargl-depth-buffer-bit-i
    // load and store actions (~clear buffer and store results for display)
    auto colorAttachment = pass->colorAttachments()->object(0);
    colorAttachment->setClearColor(MTL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    colorAttachment->setLoadAction(MTL::LoadAction::LoadActionClear);
    colorAttachment->setStoreAction(MTL::StoreAction::StoreActionStore);
    colorAttachment->setTexture(drawable->texture());

    // depth buffer
    auto depthAttachment = pass->depthAttachment();
    depthAttachment->setTexture(pDepthTexture);
    depthAttachment->setClearDepth(1.0f);
    // we will not use the depth buffer (e.g. for saving in a file) so discard it
    depthAttachment->setStoreAction(MTL::StoreAction::StoreActionDontCare);
    depthAttachment->setLoadAction(MTL::LoadAction::LoadActionClear);

    // stencil attachment
    auto stencilAttachment = pass->stencilAttachment();
    stencilAttachment->setTexture(depthAttachment->texture());
    stencilAttachment->setStoreAction(MTL::StoreAction::StoreActionDontCare);
    stencilAttachment->setLoadAction(MTL::LoadAction::LoadActionClear);


    auto encoder = buffer->renderCommandEncoder(pass);

    // TODO: this causes a crash, figure it out...
    // MTL::RenderCommandEncoder* encoder = (MTL::RenderCommandEncoder *)SDL_RenderGetMetalCommandEncoder(renderer);

    // NEED TO FIX THE VIEWPORT SIZE FOR RETINA (HIGH-DPI) DISPLAYS
    // encoder->setViewport(MTL::Viewport {
    //     0.0f, 0.0f, // origin-x, origin-y
    //     // (double)viewport[0], (double)viewport[1],
    //     (double)render_output_width, (double)render_output_height,
    //     0.0f, 1.0f // near, far
    // });

    encoder->setDepthStencilState( depth_stencil );
    encoder->setRenderPipelineState( pipeline );

    encoder->setVertexBuffer( vertex_data_buffer, /* offset */ 0, /* index */ 0 );
    encoder->setVertexBuffer( pInstanceDataBuffer, /* offset */ 0, /* index */ 1 );
    encoder->setVertexBuffer( pCameraDataBuffer, /* offset */ 0, /* index */ 2 );

    encoder->setFragmentTexture( pTexture, /* index */ 0 );

    encoder->setCullMode( MTL::CullModeBack ); // none/front/back
    encoder->setFrontFacingWinding( MTL::Winding::WindingClockwise ); // (counter)clockwise WindingCounterClockwise
    // encoder->setDepthClipMode( MTL::DepthClipMode::DepthClipModeClip ); // clip/clamp
    encoder->setTriangleFillMode( MTL::TriangleFillMode::TriangleFillModeFill ); // fill/lines TriangleFillModeFill/TriangleFillModeLines

    encoder->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle,
                                indexBufferCount, MTL::IndexType::IndexTypeUInt32,
                                index_buffer,
                                0, // offset
                                1 ); // instance count

    encoder->endEncoding();
    // encoder->release();

    // not using MTKView and therefore have to get the drawable via CAMetalLayer > nextDrawable()
    // buffer->presentDrawable( pView->currentDrawable() );
    buffer->presentDrawable(drawable);
    buffer->commit();

    // buffer->release();
    // drawable->release();

    pPool->release();

};
