#pragma once

#include <flecs.h>
#include <LLGL/LLGL.h>
#include <SDL.h>

namespace rise::rendering {
    struct CoreState {
        std::shared_ptr<LLGL::RenderSystem> renderer = nullptr;
        std::string root;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
        flecs::entity sharedState = flecs::entity(nullptr);
    };

    void importCoreState(flecs::world &ecs);
}