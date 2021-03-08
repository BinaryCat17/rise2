#pragma once

#include "util/ecs.hpp"
#include <LLGL/LLGL.h>
#include <SDL.h>
#include <functional>

namespace rise::systems::rendering {
    struct GuiState {};

    void initGui(flecs::world& ecs, LLGL::RenderSystem* renderer, SDL_Window *window,
            std::string const &path);

    void renderGui(flecs::world& ecs, LLGL::RenderSystem *renderer,
            SDL_Window *window, LLGL::CommandBuffer *cmdBuf);

    void addCallback(flecs::world& ecs, std::function<void()> const& f);
}