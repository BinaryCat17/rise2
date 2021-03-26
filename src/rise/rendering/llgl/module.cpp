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

namespace rise::rendering {
    template<typename T>
    T* getOrInit(flecs::entity e, T const& val) {
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
        ecs.component<MaterialId>("MaterialId");
        ecs.component<ViewportId>("ViewportId");
        ecs.component<ModelId>("ModelId");
        ecs.component<ApplicationId>("ApplicationId");
        ecs.component<ApplicationRef>("ApplicationRef");
        ecs.component<ViewportRef>("ViewportRef");
        ecs.component<Initialized>("Initialized");

        ecs.system<const RenderTo>("AddRegTo", "!RegTo").kind(flecs::OnSet).each(
                [](flecs::entity e, RenderTo state) {
                    auto p = state.e.get<RegTo>()->e;
                    e.set<RegTo>({p});
                });

        ecs.system<const RegTo>("AddApplicationRef").kind(flecs::OnSet).each(
                [](flecs::entity e, RegTo state) {
                    e.set<ApplicationRef>({state.e.get_ref<ApplicationId>()});
                });

        ecs.system<const RenderTo>("AddViewportRef").kind(flecs::OnSet).each(
                [](flecs::entity e, RenderTo state) {
                    e.set<ViewportRef>({state.e.get_ref<ViewportId>()});
                });

        ecs.system<>("initApplication", "Application").kind(flecs::OnAdd).each(
                [](flecs::entity e) {
                    auto title = getOrInit(e, Title{"Minecraft 2"});
                    auto path = getOrInit(e, Path{"./rendering"});

                    auto application = new ApplicationState;
                    initPlatformWindow(e, *application, *title);
                    initCoreRenderer(e, *application);
                    initPlatformSurface(e, *application);
                    initCoreState(e, *application);
                    initGuiState(e, *application, *path);
                    initSceneState(e, *application, *path);
                    e.set<ApplicationId>({application});
                    e.set<ApplicationRef>({e.get_ref<ApplicationId>()});
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

                    processRemoveInit<eTextureState>(manager, manager.texture,
                            [renderer](TextureState &state) {
                                renderer->Release(*state.val);
                            });
                    processRemoveInit<eMeshState>(manager, manager.mesh,
                            [renderer](MeshState &state) {
                                renderer->Release(*state.vertices);
                                renderer->Release(*state.indices);
                            });
                    processRemoveInit<eMaterialState>(manager, manager.material,
                            [renderer](MaterialState &state) {
                                renderer->Release(*state.uniform);
                            });
                    processRemoveInit<eViewportState>(manager, manager.viewport,
                            [renderer](ViewportState &state) {
                                renderer->Release(*state.uniform);
                            });
                    processRemoveInit<eModelState>(manager, manager.model,
                            [renderer](ModelState &state) {
                                renderer->Release(*state.uniform);
                                renderer->Release(*state.heap);
                            });
                });

        ecs.system<const ApplicationId>("recreateDescriptors").kind(flecs::PreStore).each(
                recreateDescriptors);

        ecs.system<const ApplicationId>("updateMaterial").kind(flecs::PreStore).each(
                updateMaterial);

        ecs.system<const ApplicationId>("updateTransform").kind(flecs::PreStore).each(
                updateTransform);

        ecs.system<const ApplicationRef, const ViewportId>("prepareViewport", "Initialized").
                kind(flecs::PreStore).each(prepareViewport);

        ecs.system<const ApplicationRef, const ViewportId, const Extent2D, const Position3D,
                const Rotation3D>("updateViewportCamera", "Initialized").
                kind(flecs::PreStore).each(updateViewportCamera);

        ecs.system<const ApplicationRef, const ViewportRef, const Position3D, const DiffuseColor,
                const Intensity, const Distance>("updateViewportLight", "Initialized").
                kind(flecs::PreStore).each(updateViewportLight);

        ecs.system<const ApplicationRef, const ViewportId>("finishViewport", "Initialized").
                kind(flecs::PreStore).each(finishViewport);

        ecs.system<const ApplicationId>("prepareRender", "Application").kind(flecs::PreStore).
                each(prepareRender);

        ecs.system<const ApplicationRef, const ViewportRef, const MeshId, const ModelId>(
                "renderScene", "Initialized").kind(flecs::PreStore).each(renderScene);

        ecs.system<const ApplicationId, const GuiContext>("prepareImgui", "Application").
                kind(flecs::PreStore).each(prepareImgui);

        // On store -------------------------------------------------------------------------------

        ecs.system<const GuiContext>("processImGui", "Application").kind(flecs::OnStore).each(
                processImGui);

        ecs.system<const ApplicationId, const GuiContext>("updateGuiResources", "Application").
                kind(flecs::OnStore).each(updateResources);

        ecs.system<const ApplicationId, const GuiContext, const Extent2D>("renderGui",
                "Application").kind(flecs::OnStore).each(renderGui);

        ecs.system<const ApplicationId>("submitRender", "Application").kind(flecs::OnStore).
                each(submitRender);
    }
}
