#include "material.hpp"
#include "utils.hpp"

namespace rise::rendering {
    void regMaterial(flecs::entity e) {
        if (!e.has<Albedo>()) e.set<Albedo>({1.0, 1.0f, 1.0f});
        if (!e.has<Metallic>()) e.set<Metallic>({0.0f});
        if (!e.has<Roughness>()) e.set<Roughness>({1.0f});
        e.set<MaterialId>({});
    }

    void initMaterial(flecs::entity e, ApplicationRef app, MaterialId &id) {
        if (id.id == NullKey) {
            auto &core = app.ref->id->core;

            std::tuple init{MaterialState{}, std::set<flecs::entity_t>{}};

            id.id = app.ref->id->manager.material.states.push_back(std::move(init));
            e.add_trait<Initialized, MaterialId>();
            auto uniform = createUniformBuffer<scenePipeline::PerMaterial>(core.renderer.get());
            app.ref->id->manager.material.toInit.emplace_back(MaterialState{uniform}, id);
            app.ref->id->manager.material.toUpdate.push_back(e);
        } else {
            app.ref->id->manager.material.toRemove.push_back(id);
        }
    }

    void removeMaterial(flecs::entity, ApplicationRef app, MaterialId id) {
        app.ref->id->manager.material.toRemove.push_back(id);
    }

    void unregMaterial(flecs::entity e) {
        if (e.has_trait<Initialized, MaterialId>()) {
            e.remove<MaterialId>();
            e.remove_trait<Initialized, MaterialId>();
        }
    }

    void catchMaterialUpdate(flecs::entity e, ApplicationRef app) {
            auto &manager = app.ref->id->manager;
            manager.material.toUpdate.push_back(e);
    }

    void updateMaterial(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;

        for (auto up : manager.material.toUpdate) {
            auto id = up.get<MaterialId>()->id;
            auto &material = std::get<eMaterialState>(manager.material.states.at(id)).get();

            auto color = *up.get<Albedo>();
            scenePipeline::PerMaterial data;

            data.albedo = glm::vec4(color.r, color.g, color.b, 1);
            data.metallic = up.get<Metallic>()->val;
            data.roughness = up.get<Roughness>()->val;
            if (id == app.id->presets.material.get<MaterialId>()->id) {
                data.albedo = glm::vec4(1.0f);
                data.metallic = 0.0;
                data.roughness = 1.0;
            }

            updateUniformBuffer(app.id->core.renderer.get(), material.uniform, data);
        }
    }

    void regMaterialToModel(flecs::entity e, ApplicationRef app, MaterialId material) {
        auto &manager = app.ref->id->manager;
        auto &models = std::get<eMaterialModels>(manager.material.states.at(material.id)).get();

        auto& prev = *e.get_trait_mut<Previous, MaterialId>();
        if (prev.e != flecs::entity(0)) {
            models.erase(prev.e.id());
        }

        models.insert(e.id());
        prev.e = e;
    }

    void unregMaterialFromModel(flecs::entity e, ApplicationRef app, MaterialId material) {
        auto &manager = app.ref->id->manager;
        auto &models = std::get<eMaterialModels>(manager.material.states.at(material.id)).get();
        models.erase(e.id());
    }

    void importMaterial(flecs::world &ecs) {
        ecs.system<>("regMaterial", "Material").kind(flecs::OnAdd).each(regMaterial);
        ecs.system<>("unregMaterial", "Material").kind(flecs::OnRemove).each(unregMaterial);
        ecs.system<const ApplicationRef, MaterialId>("initMaterial", "Material").
                kind(flecs::OnSet).each(initMaterial);
        ecs.system<const ApplicationRef, const MaterialId>("removeMaterial").
                kind(EcsUnSet).each(removeMaterial);
        ecs.system<const ApplicationRef, const MaterialId>("regMaterialToModel", "ModelId").
                kind(flecs::OnSet).each(regMaterialToModel);
        ecs.system<const ApplicationRef, const MaterialId>("unregMaterialFromModel", "ModelId").
                kind(EcsUnSet).each(unregMaterialFromModel);
        ecs.system<const ApplicationRef>("catchMaterialUpdate",
                "Material, TRAIT | Initialized > ModelId,"
                "[in] ANY:rise.rendering.Albedo,"
                "[in] ANY:rise.rendering.Metallic,"
                "[in] ANY:rise.rendering.Roughness").
                kind(flecs::OnSet).each(catchMaterialUpdate);
    }
}