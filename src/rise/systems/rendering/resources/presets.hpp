#pragma once
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    struct ResourcePresets {
        flecs::type mesh;
        flecs::type texture;
        flecs::type material;
        flecs::type model;
        flecs::type pointLight;
        flecs::type app;
    };
}
