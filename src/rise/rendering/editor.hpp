#pragma once
#include <flecs.h>

namespace rise::rendering {
    struct EditorComponents {
        explicit EditorComponents(flecs::world& ecs);
    };
}