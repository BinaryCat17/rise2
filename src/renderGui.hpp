#pragma once

#include <LLGL/LLGL.h>
#include <entt/entt.hpp>
#include <SDL2/SDL.h>

namespace rise {
    void initGui(entt::registry &r, LLGL::RenderSystem *renderer, SDL_Window *window,
            std::string const &path);

    void renderGui(entt::registry &r, LLGL::RenderSystem *renderer,
            SDL_Window *window, LLGL::CommandBuffer *cmdBuf);

    entt::delegate<void(entt::registry &r)> &guiCallbacks(entt::registry &r);
}