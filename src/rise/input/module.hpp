#pragma once

#include <flecs.h>

namespace rise::input {
    struct Controllable {};

    struct Module {
        explicit Module(flecs::world &ecs);
    };
}