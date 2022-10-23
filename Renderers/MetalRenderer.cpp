#include "SDL2/SDL.h"

#include "Renderers/MetalRenderer.hpp"

/*
void MetalRenderer::Initialize()
{
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_PRESENTVSYNC);

    NS::Error *err;

    mSwapChain = (CA::MetalLayer*)SDL_RenderGetMetalLayer(mRenderer);
    auto device = mSwapChain->device();

    auto name = device->name();
    printf("device name: &s\n", name->utf8String().c_str());

    auto library_data = dispatch_data_create(
        &triangle_metallib[0], triangle_metallib_len,
        dispatch_get_main_queue(),
        ^{ });

    auto library = MTL::make_owned(device->newLibrary(library_data, &err));

    if (!library) {
        std::cerr << "Failed to create library" << std::endl;
        std::exit(-1);
    }

    auto vertex_function_name = NS::String::string("vertexShader", NS::ASCIIStringEncoding);
    auto vertex_function = MTL::make_owned(library->newFunction(vertex_function_name));

    auto fragment_function_name = NS::String::string("fragmentShader", NS::ASCIIStringEncoding);
    auto fragment_function = MTL::make_owned(library->newFunction(fragment_function_name));

    auto pipeline_descriptor = MTL::make_owned(MTL::RenderPipelineDescriptor::alloc()->init());
    pipeline_descriptor->setVertexFunction(vertex_function.get());
    pipeline_descriptor->setFragmentFunction(fragment_function.get());

    auto color_attachment_descriptor = pipeline_descriptor->colorAttachments()->object(0);
    color_attachment_descriptor->setPixelFormat(mSwapChain->pixelFormat());

    auto pipeline = MTL::make_owned(device->newRenderPipelineState(pipeline_descriptor.get(), &err));

    if (!pipeline) {
        std::cerr << "Failed to create pipeline" << std::endl;
        std::exit(-1);
    }

    auto queue = MTL::make_owned(device->newCommandQueue());

}

void MetalRenderer::Update()
{
    auto drawable = mSwapChain->nextDrawable();

    auto pass = MTL::make_owned(MTL::RenderPassDescriptor::renderPassDescriptor());

    auto color_attachment = pass->colorAttachments()->object(0);
    color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
    color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
    color_attachment->setTexture(drawable->texture());

    //
    auto buffer = MTL::make_owned(queue->commandBuffer());

    //
    auto encoder = MTL::make_owned(buffer->renderCommandEncoder(pass.get()));

    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);

    encoder->setViewport(MTL::Viewport {
        0.0f, 0.0f,
        (double)width, (double)height,
        0.0f, 1.0f
        });

    encoder->setRenderPipelineState(pipeline.get());

    encoder->setVertexBytes(&triangleVertices[0], sizeof(triangleVertices), AAPLVertexInputIndexVertices);
    encoder->setVertexBytes(&viewport, sizeof(viewport), AAPLVertexInputIndexViewportSize);

    NS::UInteger vertex_start = 0, vertex_count = 3;
    encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, vertex_start, vertex_count);

    encoder->endEncoding();

    buffer->presentDrawable(drawable);
    buffer->commit();

    drawable->release();
}

void MetalRenderer::Resize(unsigned int aWidth, unsigned int aHeight)
{

}
*/
