#include "module.hpp"
#include "core/state.hpp"
#include "core/systems.hpp"
#include "core/texture.hpp"
#include "core/platform.hpp"
#include "scene/state.hpp"
#include "scene/systems.hpp"
#include "scene/mesh.hpp"
#include "scene/model.hpp"
#include "scene/material.hpp"
#include "scene/viewport.hpp"
#include "gui/state.hpp"
#include "gui/systems.hpp"

namespace rise::rendering {
    LLGLModule::LLGLModule(flecs::world &ecs) {
        ecs.module<LLGLModule>("rise::rendering::llgl");
        ecs.import<Module>();
        ecs.component<LLGLApplication>("Application");
        ecs.component<TextureRes>("TextureRes");
        ecs.component<MeshRes>("MeshRes");
        ecs.component<MaterialRes>("MaterialRes");
        ecs.component<ViewportRes>("ViewportRes");
        ecs.component<ModelRes>("ModelRes");
        ecs.component<CoreState>("CoreState");
        ecs.component<GuiState>("GuiState");
        ecs.component<SceneState>("SceneState");
        ecs.component<Presets>("Presets");

        ecs.system<const RenderTo>("AddRegTo").kind(flecs::OnSet).each(
                [](flecs::entity e, RenderTo state) {
                    auto p = state.e.get<RegTo>()->e;
                    std::cout << p.id() << std::endl;
                    e.set<RegTo>({ p });
                });

        importCoreState(ecs);
        importSceneState(ecs);
        importGuiState(ecs);
        importMesh(ecs);
        importTexture(ecs);
        importMaterial(ecs);
        importModel(ecs);
        importViewport(ecs);

        // On load --------------------------------------------------------------------------------

        ecs.system<CoreState>("pullInputEvents", "OWNED:Application").kind(flecs::OnLoad).each(
                pullInputEvents);

        // Pre store ------------------------------------------------------------------------------

        ecs.system<CoreState>("prepareRender", "OWNED:Application").kind(flecs::PreStore).
                each(prepareRender);

        ecs.system<const RegTo, const RenderTo, const MeshRes, const ModelRes>("renderScene").
                kind(flecs::PreStore).each(renderScene);

        ecs.system<CoreState, GuiState, GuiContext>("prepareImgui", "OWNED:Application").
                kind(flecs::PreStore).each(prepareImgui);

        // On store -------------------------------------------------------------------------------

        ecs.system<GuiContext>("processImGui", "OWNED:Application").kind(flecs::OnStore).each(
                processImGui);

        ecs.system<CoreState, GuiState, GuiContext>("updateGuiResources", "OWNED:Application").
                kind(flecs::OnStore).each(updateResources);

        ecs.system<CoreState, GuiState, GuiContext, const Extent2D>("renderGui",
                "OWNED:Application").kind(flecs::OnStore).each(renderGui);

        ecs.system<CoreState>("submitRender", "OWNED:Application").kind(flecs::OnStore).
                each(submitRender);
    }
}
