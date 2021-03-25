#include "material.hpp"
#include "utils.hpp"

namespace rise::rendering {
    void regMaterial(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({});
        if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0, 1.0f, 1.0f});
    }

    void initMaterial(flecs::entity e, ApplicationRef app, MaterialId &id) {
        if (!e.owns<Initialized>()) {
            auto &core = app.ref->id->core;
            auto uniform = createUniformBuffer<scenePipeline::PerMaterial>(core.renderer.get());

            std::tuple init{
                    MaterialState{uniform},
                    std::vector<flecs::entity>{}
            };

            id.id = getApp(e)->manager.material.states.push_back(std::move(init));
        }
    }

    void unregMaterial(flecs::entity e) {
        if (e.owns<Initialized>()) {
            getApp(e)->manager.material.toRemove.push_back(*e.get<MaterialId>());
            e.remove<MaterialId>();
        }
    }

    void catchMaterialUpdate(flecs::entity e, ApplicationRef app) {
        auto &manager = app.ref->id->manager;
        manager.material.toUpdate.push_back(e);
    }

    void updateMaterial(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;

        for (auto up : manager.material.toUpdate) {
            auto &material = std::get<eMaterialState>(
                    manager.material.states.at(up.get<MaterialId>()->id)).get();

            auto color = *up.get<DiffuseColor>();
            scenePipeline::PerMaterial data;
            data.diffuseColor = glm::vec4(color.r, color.g, color.b, 1);
            updateUniformBuffer(app.id->core.renderer.get(), material.uniform, data);
        }
    }

    void importMaterial(flecs::world &ecs) {
        ecs.system<>("regMaterial", "Material").kind(flecs::OnAdd).each(regMaterial);
        ecs.system<>("unregMaterial", "Material").kind(flecs::OnRemove).each(unregMaterial);
        ecs.system<const ApplicationRef, MaterialId>("initMaterial", "!Initialized").
                kind(flecs::OnSet).each(initMaterial);
        ecs.system<const ApplicationRef>("catchMaterialUpdate",
                "Material, Initialized, [in] rise.rendering.DiffuseColor").
                kind(flecs::OnSet).each(catchMaterialUpdate);
    }
}