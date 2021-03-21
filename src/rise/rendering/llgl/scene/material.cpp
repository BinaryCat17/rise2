#include "material.hpp"
#include "pipeline.hpp"
#include "../core/utils.hpp"
#include "../core/state.hpp"
#include "../module.hpp"

namespace rise::rendering {
    void regMaterial(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({});
        if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0, 1.0f, 1.0f});

        auto &core = *e.get<RegTo>()->e.get<CoreState>();
        e.set<MaterialRes>({
                createUniformBuffer<scenePipeline::PerMaterial>(core.renderer.get())
        });
    }

    void unregMaterial(flecs::entity e) {
        auto &core = *e.get<RegTo>()->e.get<CoreState>();
        auto &material = *e.get_mut<MaterialRes>();

        core.queue->WaitIdle();
        core.renderer->Release(*material.uniform);
        material.uniform = nullptr;

        e.remove<MaterialRes>();
    }

    void updateMaterial(flecs::entity, RegTo state, MaterialRes material, DiffuseColor color) {
        auto &core = *state.e.get<CoreState>();
        scenePipeline::PerMaterial data;
        data.diffuseColor = glm::vec4(color.r, color.g, color.b, 1);
        updateUniformBuffer(core.renderer.get(), material.uniform, data);
    }

    void importMaterial(flecs::world &ecs) {
        ecs.system<>("regMaterial", "Material").kind(flecs::OnAdd).each(regMaterial);
        ecs.system<>("unregMaterial", "Material").kind(flecs::OnRemove).each(unregMaterial);
        ecs.system<const RegTo, MaterialRes, const DiffuseColor>("updateMaterial", "Material").
                kind(flecs::OnSet).each(updateMaterial);
    }
}
