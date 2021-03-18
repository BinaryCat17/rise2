#pragma once
#include <flecs.h>

namespace rise::rendering {
    struct Editor {
        explicit Editor(flecs::world& ecs);
    };
}