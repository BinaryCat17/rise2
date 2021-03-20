#include "material.hpp"
#include "pipeline.hpp"
#include "../core/utils.hpp"
#include "../core/state.hpp"
#include "../module.hpp"

namespace rise::rendering {
    void regMaterial(flecs::entity e) {
        if (e.owns<Material>()) {
            if (!e.has<Path>()) e.set<Path>({});
            if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0, 1.0f, 1.0f});
            e.set<MaterialRes>({});
        }
    }

    void unregMaterial(flecs::entity e) {
        if (e.owns<Material>()) {
            e.remove<MaterialRes>();
        }
    }

    void initMaterial(flecs::entity, CoreState &core, MaterialRes &material) {
        material.uniform = createUniformBuffer<scenePipeline::PerMaterial>(core.renderer.get());
    }

    void removeMaterial(flecs::entity, CoreState &core, MaterialRes &material) {
        core.renderer->Release(*material.uniform);
    }

    void updateMaterial(flecs::entity, CoreState &core, MaterialRes material, DiffuseColor color) {
        scenePipeline::PerMaterial data;
        data.diffuseColor = glm::vec4(color.r, color.g, color.b, 1);
        updateUniformBuffer(core.renderer.get(), material.uniform, data);
    }

    void importMaterial(flecs::world &ecs) {
        ecs.system<>("regMaterial", "Material").kind(flecs::OnAdd).each(regMaterial);
        ecs.system<>("unregMaterial", "Material").kind(flecs::OnRemove).each(unregMaterial);
        ecs.system<CoreState, MaterialRes>("initMaterial", "OWNED:MaterialRes").
                kind(flecs::OnSet).each(initMaterial);
        ecs.system<CoreState, MaterialRes>("removeMaterial", "OWNED:MaterialRes").
                kind(EcsUnSet).each(removeMaterial);
        ecs.system<CoreState, MaterialRes, DiffuseColor>("updateMaterial", "OWNED:MaterialRes").
                kind(flecs::OnSet).each(updateMaterial);
    }
}
