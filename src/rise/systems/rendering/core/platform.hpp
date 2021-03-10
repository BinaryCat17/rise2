#pragma once

#include <LLGL/LLGL.h>
#include <SDL.h>
#include <glm/glm.hpp>

namespace rise {
    SDL_Window* createGameWindow(std::string const& title, glm::vec2 extent);

    LLGL::RenderContext* createRenderingContext(LLGL::RenderSystem *renderer, SDL_Window *window);

    bool pullInputEvents(SDL_Window* window);
}