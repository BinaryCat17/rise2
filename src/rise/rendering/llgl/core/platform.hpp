#pragma once

#include <LLGL/LLGL.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include "resources.hpp"
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    SDL_Window* createGameWindow(std::string const& title, glm::vec2 extent);

    LLGL::RenderContext* createRenderingContext(LLGL::RenderSystem *renderer, SDL_Window *window);

    bool pullInputEvents(flecs::entity e, CoreState& core);
}