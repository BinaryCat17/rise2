#include "model.hpp"
#include "../glm.hpp"
#include "utils.hpp"

namespace rise::rendering {
    void regModel(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});

        auto app = getApp(e);
        auto const &presets = app->presets;
        if (!e.has<MaterialId>()) e.add_instanceof(presets.material);
        if (!e.has<MeshId>()) e.add_instanceof(presets.mesh);
        if (!e.has<DiffuseTexture>()) e.set<DiffuseTexture>({presets.texture});

        auto uniform = createUniformBuffer<scenePipeline::PerObject>(app->core.renderer.get());
        e.set<ModelId>({getApp(e)->manager.model.states.push_back(
                std::tuple{ModelState{uniform}})});
    }

    void unregModel(flecs::entity e) {
        getApp(e)->manager.model.toRemove.push_back(*e.get<ModelId>());
        e.remove<ModelId>();
    }

    void clearDescriptors(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;
        auto &core = app.id->core;

        for (auto rm : manager.model.toUpdateDescriptors) {
            auto &model = std::get<eModelState>(
                    manager.model.states.at(rm.get<ModelId>()->id)).get();
            if (model.heap) {
                core.renderer->Release(*model.heap);
                model.heap = nullptr;
            }
        }
    }

    void recreateDescriptors(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;
        auto &core = app.id->core;

        for (auto up : manager.model.toUpdateDescriptors) {
            auto &model = std::get<eModelState>(
                    manager.model.states.at(up.get<ModelId>()->id)).get();
            if (!model.uniform) {
                auto const &viewport = std::get<eViewportState>(
                        manager.viewport.states.at(getViewport(up).id)).get();
                auto const &diffuse = std::get<eTextureState>(manager.texture.states.at(
                        up.get<DiffuseTexture>()->e.get<TextureId>()->id)).get();
                auto const &material = std::get<eMaterialState>(
                        manager.material.states.at(up.get<MaterialId>()->id)).get();


                LLGL::ResourceHeapDescriptor resourceHeapDesc;
                resourceHeapDesc.pipelineLayout = app.id->scene.layout;
                resourceHeapDesc.resourceViews.emplace_back(viewport.uniform);
                resourceHeapDesc.resourceViews.emplace_back(material.uniform);
                resourceHeapDesc.resourceViews.emplace_back(model.uniform);
                resourceHeapDesc.resourceViews.emplace_back(core.sampler);
                resourceHeapDesc.resourceViews.emplace_back(diffuse.val);
                model.heap = core.renderer->CreateResourceHeap(resourceHeapDesc);
            }
        }
    }

    void updateTransform(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;

        for (auto e : manager.model.toUpdateTransform) {
            auto &model = std::get<eModelState>(
                    manager.model.states.at(e.get<ModelId>()->id)).get();

            auto position = *e.get<Position3D>();
            auto rotation = *e.get<Rotation3D>();
            auto scale = *e.get<Scale3D>();

            glm::mat4 mat = glm::translate(glm::mat4(1), toGlm(position));
            float angle = std::max({rotation.x, rotation.y, rotation.z});
            if (angle != 0) {
                mat = glm::rotate(mat, glm::radians(angle), glm::normalize(toGlm(rotation)));
            }
            mat = glm::scale(mat, toGlm(scale));

            updateUniformBuffer(app.id->core.renderer.get(), model.uniform,
                    scenePipeline::PerObject{mat});
        }
    }

    void catchUpdateTransform(flecs::entity e, ApplicationRef ref) {
        ref.ref->id->manager.model.toUpdateTransform.push_back(e);
    }

    void importModel(flecs::world &ecs) {
        ecs.system<>("regModel", "Model").kind(flecs::OnAdd).each(regModel);
        ecs.system<>("unregModel", "Model").kind(flecs::OnRemove).each(unregModel);

        ecs.system<ApplicationRef>("catchUpdateTransform",
                "Model,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Scale3D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(catchUpdateTransform);
    }
}
