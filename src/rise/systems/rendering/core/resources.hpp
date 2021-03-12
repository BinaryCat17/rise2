#pragma once

#include <LLGL/LLGL.h>
#include <SDL.h>

namespace rise::systems::rendering {
    using RenderSystem = std::shared_ptr<LLGL::RenderSystem>;
    struct Window {
        SDL_Window *window = nullptr;
    };

    struct Context {
        LLGL::RenderContext *val = nullptr;
    };

    struct CommandBuffer {
        LLGL::CommandBuffer *val = nullptr;
    };

    struct Queue {
        LLGL::CommandQueue *val = nullptr;
    };

    struct Sampler {
        LLGL::Sampler *val = nullptr;
    };

    struct ResourceHeap {
        LLGL::ResourceHeap *val = nullptr;
    };

    struct Texture {
        LLGL::Texture* val = nullptr;
    };

    struct PipelineLayout {
        LLGL::PipelineLayout* val = nullptr;
    };

    struct Pipeline {
        LLGL::PipelineState* val = nullptr;
    };

    using VertexFormat = LLGL::VertexFormat;

    struct Mesh {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };

    struct CoreTag {};
}
