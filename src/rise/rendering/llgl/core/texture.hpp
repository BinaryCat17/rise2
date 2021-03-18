#pragma once

#include <flecs/flecs.hpp>
#include "../module.hpp"
#include "resources.hpp"

namespace rise::rendering {
    void updateTexture(flecs::entity e, CoreState& core, TextureRes &texture, Path const &path);
}
