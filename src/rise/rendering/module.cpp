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
        ecs.component<DiffuseTexture>("DiffuseTexture");
        ecs.component<GuiContext>("GuiContext");
    }

    void regMesh(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regMesh(app, e);
    }

    void regTexture(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regTexture(app, e);
    }

    void regMaterial(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regMaterial(app, e);
    }

    void regModel(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regModel(app, e);
    }

    void regPointLight(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regPointLight(app, e);
    }

    void regViewport(flecs::entity app, flecs::entity e) {
        (*app.get<Application>())->regViewport(app, e);
    }
}