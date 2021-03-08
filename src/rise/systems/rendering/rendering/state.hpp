#pragma once
#include <LLGL/LLGL.h>
#include <SDL.h>

namespace rise::systems::rendering {
    struct RenderState {
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
    };

    RenderState createRenderState(std::string const &root, SDL_Window *window);
}
