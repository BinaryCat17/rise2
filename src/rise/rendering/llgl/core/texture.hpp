#pragma once

#include <flecs.h>
#include "../module.hpp"
#include "resources.hpp"

namespace rise::rendering {
    void updateTexture(flecs::entity e, CoreState& core, TextureRes &texture, Path const &path);
}
