#pragma once

#include "util/ecs.hpp"

namespace rise::systems::rendering {
    struct Dirty {};

    void resetDirtySystem(flecs::iter it);
}