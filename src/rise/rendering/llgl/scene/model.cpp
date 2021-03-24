#include "model.hpp"

namespace rise::rendering {
     ModelRes createResourceHeap(* diffuse, MaterialRes material) {
        auto &core = *state.e.get<CoreState>();
        auto &scene = *state.e.get<SceneState>();
        auto &viewport = *target.e.get<ViewportRes>();

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

    void updateTransform(flecs::entity, RegTo state, ModelRes &model,
            Position3D position, Rotation3D rotation, Scale3D scale) {
        auto &core = *state.e.get<CoreState>();

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

        ecs.system<const RegTo, const RenderTo, ModelRes, const DiffuseTexture, const MaterialRes>(
                "updateModelResourceHeap").kind(flecs::OnSet).each(updateResourceHeap);

        ecs.system<const RegTo, ModelRes, const Position3D, const Rotation3D, const Scale3D>(
                "updateModelTransform").kind(flecs::OnSet).each(updateTransform);
    }
}
