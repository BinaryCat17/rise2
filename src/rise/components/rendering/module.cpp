#include "module.hpp"
#include "flecs_meta.h"

namespace rise::components {
    using namespace rendering;

    Rendering::Rendering(flecs::world &ecs) {
        ecs.import<flecs::components::meta>();
        ecs.module<Rendering>("rise.components.rendering");
        ecs.component<Extent2D>("Extent2D");
        flecs::meta<Extent2D>(ecs);
        ecs.component<Position2D>("Position2D");
        flecs::meta<Position2D>(ecs);
        ecs.component<Rotation2D>("Rotation2D");
        flecs::meta<Rotation2D>(ecs);
        ecs.component<Scale2D>("Scale3D");
        flecs::meta<Scale2D>(ecs);
        ecs.component<Extent3D>("Extent3D");
        flecs::meta<Extent3D>(ecs);
        ecs.component<Position3D>("Position3D");
        flecs::meta<Position3D>(ecs);
        ecs.component<Rotation3D>("Rotation3D");
        flecs::meta<Rotation3D>(ecs);
        ecs.component<Scale3D>("Scale3D");
        flecs::meta<Scale3D>(ecs);
        ecs.component<DiffuseColor>("DiffuseColor");
        flecs::meta<DiffuseColor>(ecs);
        ecs.component<Distance>("Distance");
        flecs::meta<Distance>(ecs);
        ecs.component<Intensity>("Intensity");
        flecs::meta<Intensity>(ecs);
        ecs.component<Path>("Path");
        flecs::meta<Path>(ecs);
    }
}