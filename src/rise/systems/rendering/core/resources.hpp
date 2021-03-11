#pragma once

#include <LLGL/LLGL.h>
#include <SDL.h>

namespace rise::systems::rendering {
    using RenderSystem = std::unique_ptr<LLGL::RenderSystem>;
    using Window = SDL_Window*;
    using Context = LLGL::RenderContext*;
    using CommandBuffer = LLGL::CommandBuffer*;
    using Queue = LLGL::CommandQueue*;
    using Sampler = LLGL::Sampler*;
    using ResourceHeap = LLGL::ResourceHeap *;
    using Texture = LLGL::Texture *;
    using PipelineLayout = LLGL::PipelineLayout*;
    using Pipeline = LLGL::PipelineState*;
    using VertexFormat = LLGL::VertexFormat;

    struct MeshRes {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };
}
