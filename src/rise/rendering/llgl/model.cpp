#include "model.hpp"
#include "../glm.hpp"
#include "utils.hpp"

namespace rise::rendering {
    void regModel(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});
        e.set<ModelId>({});
        e.set_trait<Previous, MaterialId>({flecs::entity(0)});
        e.set_trait<Previous, TextureId>({flecs::entity(0)});
        e.set_trait<Previous, ViewportId>({flecs::entity(0)});
        e.set_trait<Previous, MeshId>({flecs::entity(0)});
        e.set_trait<Previous, ModelId>({flecs::entity(0)});
    }

    void initModel(flecs::entity e, ApplicationRef ref, ModelId &id) {
        auto app = ref.ref->id;
        if (!e.has_trait<Initialized, ModelId>()) {
            auto const &presets = app->presets;
            if (!e.has<MaterialId>()) e.add_instanceof(presets.material);
            if (!e.has<MeshId>()) e.add_instanceof(presets.mesh);
            if (!e.has<AlbedoTexture>()) e.set<AlbedoTexture>({presets.texture});
            if (!e.has<MetallicTexture>()) e.set<MetallicTexture>({presets.texture});
            if (!e.has<RoughnessTexture>()) e.set<RoughnessTexture>({presets.texture});

            auto uniform = createUniformBuffer<scenePipeline::PerObject>(
                    app->core.renderer.get());
            id.id = app->manager.model.states.push_back(
                    std::tuple{ModelState{uniform}, std::set<flecs::entity_t>{}});
            e.add_trait<Initialized, ModelId>();
            app->manager.model.toUpdateTransform.push_back(e.id());
            app->manager.model.toUpdateDescriptors.push_back(e.id());

            for (auto &&light : app->manager.light.states) {
                auto &model = std::get<eLightShadowModels>(light).get();
                model.emplace(id.id, ShadowModel{});
                app->manager.light.toInitShadowModels.push_back(id);
            }
            e.set<ModelInitialized>({true});
        }
    }

    void unregModel(flecs::entity e) {
        if (e.has_trait<Initialized, ModelId>()) {
            e.remove<ModelId>();
            e.remove_trait<Initialized, ModelId>();
            e.remove<ModelInitialized>();
        }
    }

    void removeModel(flecs::entity, ApplicationRef ref, ModelId id) {
        auto app = ref.ref->id;
        app->manager.model.toRemove.push_back(id);
        app->manager.light.toRemoveShadowModels.push_back(id);
    }

    void clearDescriptors(flecs::entity e, ApplicationId app) {
        auto &manager = app.id->manager;
        auto &core = app.id->core;

        for (auto irm : manager.model.toUpdateDescriptors) {
            flecs::entity rm(e.world(), irm);
            auto &model = std::get<eModelState>(
                    manager.model.states.at(rm.get<ModelId>()->id)).get();
            if (model.heap) {
                core.renderer->Release(*model.heap);
                model.heap = nullptr;
            }
        }
    }

    template<typename T>
    TextureId getTexId(flecs::entity up, flecs::entity e, ApplicationId app) {
        auto diffuseE = up.get<T>()->e;
        TextureId diffuseId;
        if (auto p = diffuseE.template get<TextureId>()) {
            diffuseId = *p;
        } else {
            diffuseId = *app.id->presets.texture.get<TextureId>();
            up.mut(e).remove<T>();
        }
        return diffuseId;
    }

    void recreateDescriptors(flecs::entity e, ApplicationId app) {
        auto &manager = app.id->manager;
        auto &core = app.id->core;

        for (auto iup : manager.model.toUpdateDescriptors) {
            flecs::entity up(e.world(), iup);
            auto &model = std::get<eModelState>(
                    manager.model.states.at(up.get<ModelId>()->id)).get();
            if (!model.heap) {
                auto const &viewport = std::get<eViewportState>(
                        manager.viewport.states.at(getViewport(up).id)).get();
                TextureId diffuseId = getTexId<AlbedoTexture>(up, e, app);
                TextureId metallicId = getTexId<MetallicTexture>(up, e, app);
                TextureId roughnessId = getTexId<RoughnessTexture>(up, e, app);

                auto const &diffuse = std::get<eTextureState>(
                        manager.texture.states.at(diffuseId.id)).get();
                auto const &metallic = std::get<eTextureState>(
                        manager.texture.states.at(metallicId.id)).get();
                auto const &roughness = std::get<eTextureState>(
                        manager.texture.states.at(roughnessId.id)).get();

                auto const &material = std::get<eMaterialState>(
                        manager.material.states.at(up.get<MaterialId>()->id)).get();

                LLGL::ResourceHeapDescriptor resourceHeapDesc;
                resourceHeapDesc.pipelineLayout = app.id->scene.layout;
                resourceHeapDesc.resourceViews.emplace_back(viewport.uniform);
                resourceHeapDesc.resourceViews.emplace_back(material.uniform);
                resourceHeapDesc.resourceViews.emplace_back(model.uniform);
                resourceHeapDesc.resourceViews.emplace_back(core.sampler);
                resourceHeapDesc.resourceViews.emplace_back(diffuse.val);
                resourceHeapDesc.resourceViews.emplace_back(metallic.val);
                resourceHeapDesc.resourceViews.emplace_back(roughness.val);
                resourceHeapDesc.resourceViews.emplace_back(viewport.cubeMaps);
                resourceHeapDesc.resourceViews.emplace_back(app.id->shadows.sampler);
                model.heap = core.renderer->CreateResourceHeap(resourceHeapDesc);
            }
        }
    }

    void updateTransform(flecs::entity e, ApplicationId app) {
        auto &manager = app.id->manager;

        for (auto iup : manager.model.toUpdateTransform) {
            flecs::entity up(e.world(), iup);
            if (up.has_trait<Initialized, ModelId>()) {
                auto &model = std::get<eModelState>(
                        manager.model.states.at(up.get<ModelId>()->id)).get();

                auto position = *getOrDefault(up, Position3D{0, 0, 0});
                auto rotation = *getOrDefault(up, Rotation3D{0, 0, 0});
                auto scale = *getOrDefault(up, Scale3D{1, 1, 1});

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
    }

    void catchUpdateTransform(flecs::entity e, ApplicationRef ref) {
        ref.ref->id->manager.model.toUpdateTransform.push_back(e.id());
    }

    void regModelToViewport(flecs::entity e, ApplicationRef app, ViewportRef viewport) {
        if (viewport.ref->id != NullKey) {
            auto &manager = app.ref->id->manager;
            auto &models = std::get<eViewportModels>(
                    manager.viewport.states.at(viewport.ref->id)).get();

            auto &prev = *e.get_trait_mut<Previous, ViewportId>();
            if (prev.e != flecs::entity(0)) {
                models.erase(prev.e.id());
            }

            models.insert(e.id());
            prev.e = e;
        }
    }

    void unregModelFromViewport(flecs::entity e, ApplicationRef app, ViewportRef viewport) {
        if (viewport.ref->id != NullKey) {
            auto &manager = app.ref->id->manager;
            auto &models = std::get<eViewportModels>(
                    manager.viewport.states.at(viewport.ref->id)).get();
            models.erase(e.id());
        }
    }

    void importModel(flecs::world &ecs) {
        ecs.system<>("regModel", "Model").kind(flecs::OnAdd).each(regModel);
        ecs.system<>("unregModel", "Model").kind(flecs::OnRemove).each(unregModel);
        ecs.system<const ApplicationRef, ModelId>("initModel", "!TRAIT | Initialized > ModelId").
                kind(flecs::OnSet).each(initModel);
        ecs.system<const ApplicationRef, const ModelId>("removeModel").
                kind(EcsUnSet).each(removeModel);

        ecs.system<ApplicationRef>("catchUpdateTransform",
                "Model, TRAIT | Initialized > ModelId,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Scale3D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(catchUpdateTransform);

        ecs.system<const ApplicationRef, const ViewportRef>("regModelToViewport",
                "[in] ANY: ModelId").
                kind(flecs::OnSet).each(regModelToViewport);
        ecs.system<const ApplicationRef, const ViewportRef>("unregModelFromViewport",
                "[in] ANY: ModelId").
                kind(EcsUnSet).each(unregModelFromViewport);
    }
}
