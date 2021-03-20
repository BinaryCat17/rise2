#include "model.hpp"
#include "state.hpp"
#include "pipeline.hpp"
#include "../core/utils.hpp"
#include "../core/state.hpp"
#include "../core/texture.hpp"
#include "../module.hpp"
#include "rendering/glm.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "viewport.hpp"

namespace rise::rendering {
    void regModel(flecs::entity e) {
        if (e.owns<Model>()) {
            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});

            e.set<ModelRes>({});
        }

    }

    void unregModel(flecs::entity e) {
        if (e.owns<Model>()) {
            e.remove<ModelRes>();
        }
    }

    void initModel(flecs::entity e, CoreState &core, Presets &presets, ModelRes &model) {
        if (!e.has<MaterialRes>()) e.add_instanceof(presets.material);
        if (!e.has<MeshRes>()) e.add_instanceof(presets.mesh);
        if (!e.has<DiffuseTexture>()) e.set<DiffuseTexture>({presets.texture});

        model.uniform = createUniformBuffer<scenePipeline::PerObject>(core.renderer.get());
    }

    void removeModel(flecs::entity, CoreState &core, ModelRes &model) {
        core.renderer->Release(*model.heap);
        core.renderer->Release(*model.uniform);
    }

    void updateResourceHeap(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            DiffuseTexture diffuse, MaterialRes material, ViewportRes viewport) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = scene.layout;
        resourceHeapDesc.resourceViews.emplace_back(viewport.uniform);
        resourceHeapDesc.resourceViews.emplace_back(material.uniform);
        resourceHeapDesc.resourceViews.emplace_back(model.uniform);
        resourceHeapDesc.resourceViews.emplace_back(core.sampler);
        resourceHeapDesc.resourceViews.emplace_back(diffuse.e.get<TextureRes>()->val);

        if (model.heap) {
            core.renderer->Release(*model.heap);
        }

        model.heap = core.renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void updateTransform(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            Position3D position, Rotation3D rotation, Scale3D scale) {
        glm::mat4 mat = glm::translate(glm::mat4(1), toGlm(position));
        float angle = std::max({rotation.x, rotation.y, rotation.z});
        if (angle != 0) {
            mat = glm::rotate(mat, glm::radians(angle), glm::normalize(toGlm(rotation)));
        }
        mat = glm::scale(mat, toGlm(scale));

        updateUniformBuffer(core.renderer.get(), model.uniform, scenePipeline::PerObject{mat});
    }

    void importModel(flecs::world &ecs) {
        ecs.system<>("regModel", "Model").kind(flecs::OnAdd).each(regModel);
        ecs.system<>("unregModel", "Model").kind(flecs::OnRemove).each(unregModel);
        ecs.system<CoreState, Presets, ModelRes>("initModel", "OWNED:ModelRes").
                kind(flecs::OnSet).each(initModel);
        ecs.system<CoreState, ModelRes>("removeModel", "OWNED:ModelRes").
                kind(EcsUnSet).each(removeModel);

        ecs.system<CoreState, SceneState, ModelRes, const DiffuseTexture, const MaterialRes,
                const ViewportRes>("updateModelResourceHeap", "OWNED:ModelRes").
                kind(flecs::OnSet).each(updateResourceHeap);

        ecs.system<CoreState, SceneState, ModelRes, const Position3D, const Rotation3D,
                const Scale3D>("updateModelTransform", "OWNED:ModelRes").
                kind(flecs::OnSet).each(updateTransform);
    }
}
