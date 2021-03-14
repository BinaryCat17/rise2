#include "module.hpp"
#include "flecs_meta.h"
#include "imgui.hpp"

namespace rise::components {
    using namespace rendering;

    Rendering::Rendering(flecs::world &ecs) {
        ecs.module<Rendering>("rise::components::rendering");
        flecs::meta<Relative>(ecs);
        flecs::meta<Extent2D>(ecs);
        flecs::meta<Position2D>(ecs);
        flecs::meta<Rotation2D>(ecs);
        flecs::meta<Scale2D>(ecs);
        flecs::meta<Extent3D>(ecs);
        flecs::meta<Position3D>(ecs);
        flecs::meta<Rotation3D>(ecs);
        flecs::meta<Scale3D>(ecs);
        flecs::meta<DiffuseColor>(ecs);
        flecs::meta<Distance>(ecs);
        flecs::meta<Intensity>(ecs);
        flecs::meta<Path>(ecs);
        ecs.component<GuiContext>("GuiContext");
    }
}