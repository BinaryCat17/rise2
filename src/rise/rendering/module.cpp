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
        ecs.component<Albedo>("Albedo");
        ecs.component<Metallic>("Metallic");
        ecs.component<Ao>("Ao");
        ecs.component<Roughness>("Roughness");
        ecs.component<Distance>("Distance");
        ecs.component<Intensity>("Intensity");
        ecs.component<Path>("Path");
        ecs.component<Relative>("Relative");
        ecs.component<Title>("Title");
        ecs.component<AlbedoTexture>("AlbedoTexture");
        ecs.component<MetallicTexture>("MetallicTexture");
        ecs.component<AoTexture>("AoTexture");
        ecs.component<RoughnessTexture>("RoughnessTexture");
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