#include "dirty.hpp"

namespace rise::systems::rendering {
    void resetDirtySystem(flecs::iter it) {
        it.world().remove<Dirty>();
    }
}


