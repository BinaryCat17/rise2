#include "module.hpp"
#include "core/systems.hpp"
#include "scene/systems.hpp"
#include "scene/pipeline.hpp"
#include "gui/systems.hpp"
#include "core/utils.hpp"
#include "core/platform.hpp"

namespace rise::rendering {
    struct Presets {
        flecs::entity material;
        flecs::entity mesh;
        flecs::entity texture;
    };

    void regApplication(flecs::entity e) {
        if (e.owns<LLGLApplication>()) {
            auto ecs = e.world();

            initCoreState(e);
            initGuiState(e);
            initSceneState(e);

            Presets presets{
                    ecs.entity().
                            add_instanceof(e).
                            set<DiffuseColor>({1.0f, 1.0f, 1.0f}).
                            add<Material>(),
                    ecs.entity().
                            add_instanceof(e).
                            set<Path>({"cube.obj"}).
                            add<Mesh>(),
                    ecs.entity().
                            add_instanceof(e).
                            set<Path>({"default.jpg"}).
                            add<Texture>(),
            };

            e.set<Presets>(presets);
            e.set<Relative>(Relative{false});
        }
    }

    void regMesh(flecs::entity e) {
        if (e.owns<Mesh>()) {
            if (!e.has<Path>()) e.set<Path>({});
            e.set<MeshRes>({});
        }
    }

    void regTexture(flecs::entity e) {
        if (e.owns<Texture>()) {
            if (!e.has<Path>()) e.set<Path>({});
            e.set<TextureRes>({});
        }
    }

    void regMaterial(flecs::entity e) {
        if (e.owns<Material>()) {
            if (!e.has<Path>()) e.set<Path>({});
            auto renderer = e.get<CoreState>()->renderer;
            if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0, 1.0f, 1.0f});
            e.set<MaterialRes>({createUniformBuffer<scenePipeline::PerMaterial>(renderer.get())});
        }
    }

    void regModel(flecs::entity e) {
        if (e.owns<Model>()) {
            auto renderer = e.get<CoreState>()->renderer;
            auto presets = e.get<Presets>();

            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});
            if (!e.has<MaterialRes>()) e.add_instanceof(presets->material);
            if (!e.has<MeshRes>()) e.add_instanceof(presets->mesh);
            if (!e.has<DiffuseTexture>()) e.set<DiffuseTexture>({presets->texture});

            e.set<ModelRes>({createUniformBuffer<scenePipeline::PerObject>(renderer.get())});
        }
    }

    void regPointLight(flecs::entity e) {
        if (e.owns<PointLight>()) {
            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Intensity>()) e.set<Intensity>({1.0f});
            if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0f, 1.0f, 1.0f});
            if (!e.has<Distance>()) e.set<Distance>({5.0f});
        }
    }

    void regViewport(flecs::entity e) {
        if (e.owns<Viewport>()) {
            auto renderer = e.get<CoreState>()->renderer;

            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
            if (!e.has<Extent2D>()) e.set<Extent2D>({0.0f, 0.0f});

            e.set<ViewportRes>({createUniformBuffer<scenePipeline::PerViewport>(renderer.get())});
        }
    }

    LLGLModule::LLGLModule(flecs::world &ecs) {
        // components
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

        // resources

        ecs.system<>("regLLGLApplication", "rise.rendering.llgl.Application").
                kind(flecs::OnAdd).each(regApplication);

        ecs.system<>("regLLGLMesh", "rise.rendering.Mesh").
                kind(flecs::OnAdd).each(regMesh);

        ecs.system<>("regTexture", "rise.rendering.Texture").
                kind(flecs::OnAdd).each(regTexture);

        ecs.system<>("regLLGLMaterial", "rise.rendering.Material").
                kind(flecs::OnAdd).each(regMaterial);

        ecs.system<>("regLLGLModel", "rise.rendering.Model").
                kind(flecs::OnAdd).each(regModel);

        ecs.system<>("regLLGLPointLight", "rise.rendering.PointLight").
                kind(flecs::OnAdd).each(regPointLight);

        ecs.system<>("regLLGLViewport", "rise.rendering.Viewport").
                kind(flecs::OnAdd).each(regViewport);

        // observers
        ecs.system<CoreState, SceneState, MeshRes, const Path>("updateMesh", "OWNED:MeshRes").
                kind(flecs::OnSet).each(updateMesh);

        ecs.system<CoreState, TextureRes, const Path>("updateTexture", "OWNED:TextureRes").
                kind(flecs::OnSet).each(updateTexture);

        ecs.system<CoreState, const Extent2D>("updateWindowSize", "OWNED:CoreState").
                kind(flecs::OnSet).each(updateWindowSize);

        ecs.system<CoreState, SceneState, ModelRes, const DiffuseTexture, const MaterialRes,
                const ViewportRes>("updateResourceHeap", "OWNED:ModelRes").
                kind(flecs::OnSet).each(updateResourceHeap);

        ecs.system<CoreState, SceneState, ModelRes, const Position3D, const Rotation3D,
                const Scale3D>("updateTransform", "OWNED:ModelRes").kind(flecs::OnSet).each(
                updateTransform);

        ecs.system<CoreState, SceneState, MaterialRes, const DiffuseColor>("updateMaterial",
                "OWNED:MaterialRes").kind(flecs::OnSet).each(updateMaterial);

        ecs.system<ViewportRes>("dirtyViewportCamera",
                "OWNED:ViewportRes,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Extent2D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(dirtyViewportCamera);

        ecs.system<ViewportRes>("dirtyViewportLight",
                "OWNED:ViewportRes,"
                "[in] rise.rendering.Position3D,"
                "[in] rise.rendering.DiffuseColor,"
                "[in] rise.rendering.Intensity,"
                "[in] rise.rendering.Distance").
                kind(flecs::OnSet).each(dirtyViewportLight);

        ecs.system<CoreState, const Relative>("updateRelative",
                "rise.rendering.Relative").kind(flecs::OnSet).each(updateRelative);

        // on load

        ecs.system<CoreState>("pullInputEvents", "OWNED:CoreState").kind(flecs::OnLoad).each(
                pullInputEvents);

        // pre store

        ecs.system<CoreState, ViewportRes>("prepareViewport", "OWNED:ViewportRes").
                kind(flecs::PreStore).each(prepareViewport);

        ecs.system<ViewportRes, const Extent2D, const Position3D, const Rotation3D>(
                        "updateViewportCamera", "OWNED:ViewportRes").kind(flecs::PreStore)
                .each(updateViewportCamera);

        ecs.system<ViewportRes, const Position3D, const DiffuseColor, const Intensity,
                const Distance>("updateViewportLight", "OWNED:ViewportRes").
                kind(flecs::PreStore).each(updateViewportLight);

        ecs.system<CoreState, ViewportRes>("finishViewport", "OWNED:ViewportRes").
                kind(flecs::PreStore).each(finishViewport);

        ecs.system<CoreState>("prepareRender", "OWNED:CoreState").kind(flecs::PreStore).
                each(prepareRender);

        ecs.system<CoreState, SceneState, const Position2D, const Extent2D, const MeshRes,
                const ModelRes>("renderScene").kind(flecs::PreStore).
                each(renderScene);

        ecs.system<CoreState, GuiState, GuiContext>("prepareImgui", "OWNED:CoreState").
                kind(flecs::PreStore).each(prepareImgui);

        // on store

        ecs.system<GuiContext>("processImGui", "OWNED:CoreState").kind(flecs::OnStore).each(
                processImGui);

        ecs.system<CoreState, GuiState, GuiContext>("updateGuiResources", "OWNED:CoreState").
                kind(flecs::OnStore).each(updateResources);

        ecs.system<CoreState, GuiState, GuiContext, const Extent2D>("renderGui",
                "OWNED:CoreState").kind(flecs::OnStore).each(renderGui);

        ecs.system<CoreState>("submitRender", "OWNED:CoreState").kind(flecs::OnStore).
                each(submitRender);
    }
}
