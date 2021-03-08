#pragma once

#include <glm/glm.hpp>
#include "util/ecs.hpp"

namespace rise::systems {
    struct rendering {
        explicit rendering(flecs::world &ecs);

        static void imGuiModule(flecs::world &ecs, std::function<void()> const &callback);
    };
}
