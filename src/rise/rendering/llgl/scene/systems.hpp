#pragma once

#include "state.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "../core/state.hpp"
#include "../module.hpp"

namespace rise::rendering {
    void renderScene(flecs::entity, RegTo state, RenderTo target, MeshRes mesh, ModelRes model);
}