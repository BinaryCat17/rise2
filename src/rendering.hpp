#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

namespace rise {
    std::unique_ptr<LLGL::RenderSystem> createRenderer();

    struct Context {
        LLGL::Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
    };

    Context createContext(LLGL::RenderSystem *renderer, unsigned width, unsigned height);

    struct ShaderResources {
        LLGL::PipelineLayout *pipelineLayout = nullptr;
        LLGL::ResourceHeap *resourcesHeap = nullptr;
        VertexInput vertex = {};
        GlobalShaderData camera = {};
    };

    ShaderResources createShaderResources(LLGL::RenderSystem *renderer);

    void bindResources(LLGL::CommandBuffer *cmdBuf, ShaderResources &resources);

    struct Pipeline {
        LLGL::ShaderProgram *shaders = nullptr;
        LLGL::PipelineState *state = nullptr;
    };

    Pipeline createPipeline(LLGL::RenderSystem *renderer, std::string const &root,
            ShaderResources const &resources);

    void bindPipeline(LLGL::CommandBuffer *cmdBuf, Pipeline& pipeline);

    template<typename FnT>
    void renderLoop(LLGL::RenderSystem *renderer, Context context, FnT &&f) {
        LLGL::CommandQueue *cmdQueue = renderer->GetCommandQueue();
        LLGL::CommandBuffer *cmdBuffer = renderer->CreateCommandBuffer();

        while (context.window->ProcessEvents()) {
            cmdBuffer->Begin();
            cmdBuffer->BeginRenderPass(*context.context);
            cmdBuffer->SetViewport(context.context->GetResolution());
            cmdBuffer->Clear(LLGL::ClearFlags::Color);
            f(cmdBuffer);
            cmdBuffer->EndRenderPass();
            cmdBuffer->End();
            cmdQueue->Submit(*cmdBuffer);
            context.context->Present();
        }
    }
}