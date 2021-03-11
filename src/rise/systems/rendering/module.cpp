#include "module.hpp"
#include "core/systems.hpp"
#include "scene/systems.hpp"
#include "gui/systems.hpp"

namespace rise::systems::rendering {
    Rendering::Rendering(flecs::world &ecs) {
        // observers
        ecs.system<const Path, const flecs::Name, const Extent2D>("initCoreState", "CoreTag").
                kind(flecs::OnAdd).each(initCoreState);


    }

    void Rendering::addApplication(flecs::entity e) {
        e.add<Application>();
        e.world().entity().add_instanceof(e).add_instanceof().add<CoreTag>();
    }

    void Rendering::addMesh(flecs::entity app, flecs::entity e) {

    }

    void Rendering::addTexture(flecs::entity app, flecs::entity e) {

    }

    void Rendering::addMaterial(flecs::entity app, flecs::entity e) {

    }

    void Rendering::addModel(flecs::entity app, flecs::entity e) {

    }

    void Rendering::addPointLight(flecs::entity app, flecs::entity e) {

    }

    void Rendering::addViewport(flecs::entity app, flecs::entity e) {

    }

    void Rendering::renderTo(flecs::entity viewport, flecs::entity e) {

    }

}
