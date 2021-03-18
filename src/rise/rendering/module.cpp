#include "module.hpp"
#include "imgui.hpp"

namespace rise::rendering {
    using namespace rendering;

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::rendering");
        ecs.component<Position2D>("Position2D");
        ecs.component<Rotation2D>("Position2D");
        ecs.component<Scale2D>("Position2D");
        ecs.component<Extent2D>("Position2D");
        ecs.component<Position3D>("Position3D");
        ecs.component<Rotation3D>("Position3D");
        ecs.component<Scale3D>("Position3D");
        ecs.component<Extent3D>("Position3D");
        ecs.component<DiffuseColor>("DiffuseColor");
        ecs.component<Distance>("Distance");
        ecs.component<Intensity>("Intensity");
        ecs.component<Path>("Path");
        ecs.component<Relative>("Relative");
        ecs.component<DiffuseTexture>("DiffuseTexture");
        ecs.component<GuiContext>("GuiContext");
    }
}