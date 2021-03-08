#include "module.hpp"

namespace rise::components {
    using namespace rendering;

    Rendering::Rendering(flecs::world &ecs) {
        ecs.module<Rendering>("rise.components.rendering");
        ecs.component<Extent2D>("Extent2D");
        ecs.component<Position2D>("Position2D");
        ecs.component<Rotation2D>("Rotation2D");
        ecs.component<Scale2D>("Scale3D");
        ecs.component<Extent3D>("Extent3D");
        ecs.component<Position3D>("Position3D");
        ecs.component<Rotation3D>("Rotation3D");
        ecs.component<Scale3D>("Scale3D");
        ecs.component<DiffuseColor>("DiffuseColor");
        ecs.component<Distance>("Distance");
        ecs.component<Intensity>("Intensity");
        ecs.component<Path>("Path");
        ecs.component<Mesh>("Mesh");
        ecs.component<Texture>("Texture");
        ecs.component<Material>("Material");
        ecs.component<Model>("Model");
        ecs.component<PointLight>("PointLight");
        ecs.component<Viewport>("Viewport");
        ecs.component<Application>("Application");
    }
}