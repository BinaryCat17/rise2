#pragma once
#include "rise/util/ecs.hpp"

namespace rise::systems {

    struct Input  {
        explicit Input(flecs::world& ecs);

        struct Controllable {};
    };
}