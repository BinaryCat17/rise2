#pragma once

#include "components/rendering/module.hpp"
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    struct Rendering {
        explicit Rendering(flecs::world &ecs);

        void addApplication(flecs::entity e);

        void addMesh(flecs::entity app, flecs::entity e);

        void addTexture(flecs::entity app, flecs::entity e);

        void addMaterial(flecs::entity app, flecs::entity e);

        void addModel(flecs::entity app, flecs::entity e);

        void addPointLight(flecs::entity app, flecs::entity e);

        void addViewport(flecs::entity app, flecs::entity e);

        void renderTo(flecs::entity viewport, flecs::entity e);
    };
}
