#pragma once

#include <LLGL/LLGL.h>
#include <SDL.h>

namespace rise::systems::rendering {
    struct CoreState {
        std::shared_ptr<LLGL::RenderSystem> renderer = nullptr;
        std::string path;
        SDL_Window *window = nullptr;
        LLGL::RenderContext* context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
    };

    struct TextureRes {
        LLGL::Texture *val = nullptr;
    };

    struct MeshRes {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };
}
