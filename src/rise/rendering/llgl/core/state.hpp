#pragma once

#include <flecs.h>
#include <LLGL/LLGL.h>
#include <SDL.h>
#include "util/soa.hpp"

namespace rise::rendering {
    struct CoreState {
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
    };

    void importCoreState(flecs::world &ecs);
}