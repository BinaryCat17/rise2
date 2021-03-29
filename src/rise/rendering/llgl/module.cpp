#include "module.hpp"
#include "core.hpp"
#include "gui.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "platform.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "viewport.hpp"
#include "shadows.hpp"

namespace rise::rendering {
    template<typename T>
    T *getOrInit(flecs::entity e, T const &val) {
        bool is_added;
        auto m = e.get_mut<T>(&is_added);
        if (is_added) {
            *m = val;
        }
        return m;
    }

    LLGLModule::LLGLModule(flecs::world &ecs) {
        ecs.module<LLGLModule>("rise::rendering::llgl");
        ecs.import<Module>();
        ecs.component<LLGLApplication>("Application");
        ecs.component<TextureId>("TextureId");
        ecs.component<MeshId>("MeshId");
        ecs.component<LightId>("LightId");
        ecs.component<MaterialId>("MaterialId");
        ecs.component<ViewportId>("ViewportId");
        ecs.component<ModelId>("ModelId");
        ecs.component<ApplicationId>("ApplicationId");
        ecs.component<ApplicationRef>("ApplicationRef");
        ecs.component<ViewportRef>("ViewportRef");
        ecs.component<Initialized>("Initialized");
        ecs.component<Previous>("Previous");

        ecs.system<const RenderTo>("AddRegTo", "!RegTo").kind(flecs::OnSet).each(
                [](flecs::entity e, RenderTo state) {
                    auto p = state.e.get<RegTo>()->e;
                    e.set<RegTo>({p});
                });

        ecs.system<const RegTo>("AddApplicationRef").kind(flecs::OnSet).each(
                [](flecs::entity e, RegTo state) {
                    if (state.e.has<ApplicationId>()) {
                        e.set<ApplicationRef>({state.e.get_ref<ApplicationId>()});
                    }
                });

        ecs.system<const RenderTo>("AddViewportRef").kind(flecs::OnSet).each(
                [](flecs::entity e, RenderTo state) {
                    e.set<ViewportRef>({state.e.get_ref<ViewportId>()});
                });

        ecs.system<>("initApplication", "Application").kind(flecs::OnAdd).each(
                [](flecs::entity e) {
                    auto title = getOrInit(e, Title{"Minecraft 2"});
                    auto path = getOrInit(e, Path{"./rendering"});
                    auto extent = getOrInit(e, Extent2D{1600, 1000});
                    e.set<Relative>({false});

                    auto application = new ApplicationState;
                    initPlatformWindow(e, *application, *title, *extent);
                    initCoreRenderer(e, *application);
                    initPlatformSurface(e, *application);
                    initCoreState(e, *application);
                    initGuiState(e, *application, *path);
                    initSceneState(e, *application, *path);
                    e.set<ApplicationId>({application});
                    e.set<ApplicationRef>({e.get_ref<ApplicationId>()});
                });

        ecs.system<ApplicationId>("initApplicationPresets").kind(flecs::OnSet).each(
                [](flecs::entity e, ApplicationId app) {
                    auto ecs = e.world();
                    flecs::filter filter(ecs);
                    filter.include<RegTo>();

                    for (auto it : ecs.filter(filter)) {
                        for (auto row : it) {
                            auto p = it.entity(row);
                            auto ref = e.get_ref<ApplicationId>();
                            if (ref.entity() == e) {
                                p.mut(it).set<ApplicationRef>({ref});
                            }
                        }
                    }
                });

        ecs.system<>("removeApplication", "Application").kind(flecs::OnRemove).each(
                [](flecs::entity e) {
                    delete e.get<ApplicationId>();
                    e.remove<ApplicationId>();
                });

        importCoreState(ecs);
        importPlatformState(ecs);
        importSceneState(ecs);
        importGuiState(ecs);
        importMesh(ecs);
        importTexture(ecs);
        importMaterial(ecs);
        importModel(ecs);
        importShadowsState(ecs);
        importViewport(ecs);

        // On load --------------------------------------------------------------------------------

        ecs.system<const ApplicationId, Extent2D>("pullInputEvents", "Application").
                kind(flecs::OnLoad).each(pullInputEvents);

        // Pre store ------------------------------------------------------------------------------

        ecs.system<const ApplicationId>("prepareResourcesRemove").kind(flecs::PreStore).each(
                [](flecs::entity e, ApplicationId app) {
                    auto &manager = app.id->manager;
                    prepareRemove<eTextureModels>(manager, manager.texture);
                    prepareRemove<eMaterialModels>(manager, manager.material);
                    prepareRemove<eViewportModels>(manager, manager.viewport);
                });

        ecs.system<const ApplicationId>("clearDescriptors").kind(flecs::PreStore).each(
                clearDescriptors);

        ecs.system<const ApplicationId>("processResourcesRemove").kind(flecs::PreStore).each(
                [](flecs::entity e, ApplicationId app) {
                    auto &manager = app.id->manager;
                    auto renderer = app.id->core.renderer.get();
                    auto queue = app.id->core.queue;

                    processRemoveInit<eTextureState>(manager, manager.texture,
                            [renderer, queue](TextureState &state) {
                                if (state.val != nullptr) {
                                    queue->WaitIdle();
                                    renderer->Release(*state.val);
                                }
                            });
                    processRemoveInit<eMeshState>(manager, manager.mesh,
                            [renderer, queue](MeshState &state) {
                                if (state.vertices != nullptr) {
                                    queue->WaitIdle();
                                    renderer->Release(*state.vertices);
                                    renderer->Release(*state.indices);
                                }
                            });
                    processRemoveInit<eMaterialState>(manager, manager.material,
                            [renderer, queue](MaterialState &state) {
                                if (state.uniform != nullptr) {
                                    queue->WaitIdle();
                                    renderer->Release(*state.uniform);
                                }
                            });
                    processRemoveInit<eViewportState>(manager, manager.viewport,
                            [renderer, queue](ViewportState &state) {
                                queue->WaitIdle();
                                renderer->Release(*state.uniform);
                            });
                    processRemoveInit<eModelState>(manager, manager.model,
                            [renderer, queue](ModelState &state) {
                                queue->WaitIdle();
                                renderer->Release(*state.uniform);
                                renderer->Release(*state.heap);
                            });
                    processRemoveInit<eLightState>(manager, manager.light,
                            [renderer, queue](LightState &state) {
                                queue->WaitIdle();
                                renderer->Release(*state.uniform);
                            });
                });



        ecs.system<const ApplicationId>("recreateDescriptors").kind(flecs::PreStore).each(
                recreateDescriptors);

        ecs.system<const ApplicationId>("updateMaterial").kind(flecs::PreStore).each(
                updateMaterial);

        ecs.system<const ApplicationId>("updateTransform").kind(flecs::PreStore).each(
                updateTransform);

        ecs.system<const ApplicationRef, const ViewportId>("prepareViewport",
                "TRAIT | Initialized > ViewportId").
                kind(flecs::PreStore).each(prepareViewport);

        ecs.system<const ApplicationRef, const ViewportId, const Extent2D, const Position3D,
                const Rotation3D>("updateViewportCamera", "TRAIT | Initialized > ViewportId").
                kind(flecs::PreStore).each(updateViewportCamera);

        ecs.system<const ApplicationRef, const ViewportRef, const Position3D, const DiffuseColor,
                const Intensity, const Distance>("updateViewportLight", "PointLight").
                kind(flecs::PreStore).each(updateViewportLight);

        ecs.system<const ApplicationRef, const ViewportId>("finishViewport",
                "TRAIT | Initialized > ViewportId").
                kind(flecs::PreStore).each(finishViewport);

        ecs.system<const ApplicationId>("prepareRender", "Application").kind(flecs::PreStore).
                each(prepareRender);

        ecs.system<const ApplicationId>("colorPass", "Application").kind(flecs::PreStore).
                each(prepareColorPass);

        ecs.system<const ApplicationRef, const ViewportRef, const MeshId, const ModelId>(
                "renderScene",
                "ANY: TRAIT | Initialized > MeshId," "ANY: TRAIT | Initialized > ModelId"
        ).kind(flecs::PreStore).each(renderScene);

        ecs.system<const ApplicationId, const GuiContext>("prepareImgui", "Application").
                kind(flecs::PreStore).each(prepareImgui);

        // On store -------------------------------------------------------------------------------

        ecs.system<const GuiContext>("processImGui", "Application").kind(flecs::OnStore).each(
                processImGui);

        ecs.system<const ApplicationId, const GuiContext>("updateGuiResources", "Application").
                kind(flecs::OnStore).each(updateResources);

        ecs.system<const ApplicationId, const GuiContext, const Extent2D>("renderGui",
                "OWNED:Application").kind(flecs::OnStore).each(renderGui);

        ecs.system<const ApplicationId>("endColorPass", "Application").kind(flecs::OnStore).
                each(endColorPass);

        ecs.system<const ApplicationId>("submitRender", "Application").kind(flecs::OnStore).
                each(submitRender);

        ecs.system<const ApplicationId>("clearCommands").kind(flecs::OnStore).each(
                [](flecs::entity e, ApplicationId app) {
                    auto &manager = app.id->manager;
                    manager.texture.toInit.clear();
                    manager.texture.toRemove.clear();
                    manager.viewport.toInit.clear();
                    manager.viewport.toRemove.clear();
                    manager.mesh.toInit.clear();
                    manager.mesh.toRemove.clear();
                    manager.model.toInit.clear();
                    manager.model.toUpdateDescriptors.clear();
                    manager.model.toUpdateTransform.clear();
                    manager.model.toRemove.clear();
                    manager.material.toRemove.clear();
                    manager.material.toUpdate.clear();
                    manager.material.toInit.clear();
                });
    }
}
