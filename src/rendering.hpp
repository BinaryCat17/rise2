#pragma once
#include <LLGL/LLGL.h>

namespace rise {
    std::unique_ptr<LLGL::RenderSystem> createRenderer();

    LLGL::RenderContext *createContext(LLGL::RenderSystem *renderer, unsigned width,
            unsigned height);

    std::pair<LLGL::Buffer *, LLGL::VertexFormat> createVertexBuffer(LLGL::RenderSystem *renderer);

    LLGL::ShaderProgram *createShaderProgram(LLGL::RenderSystem *renderer, std::string const &root,
            LLGL::VertexFormat const& format);

    std::pair<LLGL::PipelineState *, LLGL::PipelineLayout *> createPipeline(
            LLGL::RenderSystem *renderer, LLGL::ShaderProgram *program);

    LLGL::ResourceHeap *createResources(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout* layout);
}