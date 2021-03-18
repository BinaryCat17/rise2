#pragma once

#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateTexture(flecs::entity e, CoreState& core, TextureRes &texture, Path const &path);
}
