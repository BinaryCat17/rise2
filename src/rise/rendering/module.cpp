#include "module.hpp"
#include "imgui.hpp"

namespace rise::rendering {
    using namespace rendering;

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::rendering");
        ecs.component<Position2D>("Position2D");
        ecs.component<Rotation2D>("Rotation2D");
        ecs.component<Scale2D>("Scale2D");
        ecs.component<Extent2D>("Extent2D");
        ecs.component<Position3D>("Position3D");
        ecs.component<Rotation3D>("Rotation3D");
        ecs.component<Scale3D>("Scale3D");
        ecs.component<Extent3D>("Extent3D");
        ecs.component<DiffuseColor>("DiffuseColor");
        ecs.component<Distance>("Distance");
        ecs.component<Intensity>("Intensity");
        ecs.component<Path>("Path");
        ecs.component<Relative>("Relative");
        ecs.component<Title>("Title");
        ecs.component<DiffuseTexture>("DiffuseTexture");
        ecs.component<GuiContext>("GuiContext");
        ecs.component<Mesh>("Mesh");
        ecs.component<Texture>("Texture");
        ecs.component<Material>("Material");
        ecs.component<Model>("Model");
        ecs.component<Shadow>("Shadow");
        ecs.component<PointLight>("PointLight");
        ecs.component<Viewport>("Viewport");
        ecs.component<RegTo>("RegTo");
        ecs.component<RenderTo>("RenderTo");
    }
}