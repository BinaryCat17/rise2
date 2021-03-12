#pragma once

#include "rise/components/rendering/module.hpp"
#include "rise/util/ecs.hpp"

namespace rise::systems {
    struct Rendering {
        explicit Rendering(flecs::world &ecs);

        static void regApplication(flecs::entity e);

        static void regMesh(flecs::entity app, flecs::entity e);

        static void regTexture(flecs::entity app, flecs::entity e);

        static void regMaterial(flecs::entity app, flecs::entity e);

        static void regModel(flecs::entity app, flecs::entity e);

        static void regPointLight(flecs::entity app, flecs::entity e);

        static void regViewport(flecs::entity app, flecs::entity e);
    };
}
