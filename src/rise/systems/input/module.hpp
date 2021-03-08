#pragma once
#include <glm/glm.hpp>
#include <flecs.h>

struct SDL_Window;

namespace rise {
    struct InputModule {
        explicit InputModule(flecs::world& ecs);
    };
}