#include "module.hpp"
#include "core/systems.hpp"
#include "scene/systems.hpp"
#include "scene/pipeline.hpp"
#include "gui/systems.hpp"
#include "core/utils.hpp"
#include "core/platform.hpp"

namespace rise::rendering {
    LLGLModule::LLGLModule(flecs::world &ecs) {
        // components
        ecs.module<LLGLModule>("rise::rendering::LLGL");
        ecs.import<Module>();
        ecs.component<TextureRes>("TextureRes");
        ecs.component<MeshRes>("MeshRes");
        ecs.component<MaterialRes>("MaterialRes");
        ecs.component<ViewportRes>("ViewportRes");
        ecs.component<ModelRes>("ModelRes");
        ecs.component<CoreState>("CoreState");
        ecs.component<GuiState>("GuiState");
        ecs.component<SceneState>("SceneState");

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

        auto e = ecs.entity().add<Relative>();

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

    class LLGLApplication : public ApplicationT {
    public:
        LLGLApplication(const flecs::entity &meshPreset, const flecs::entity &texturePreset,
                const flecs::entity &materialPreset) : meshPreset(meshPreset),
                texturePreset(texturePreset), materialPreset(materialPreset) {}

        void regMesh(flecs::entity app, flecs::entity e) override {
            assert(e.has<Path>());
            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<MeshRes>({});
        }

        void regTexture(flecs::entity app, flecs::entity e) override {
            assert(e.has<Path>());
            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<TextureRes>({});
        }

        void regMaterial(flecs::entity app, flecs::entity e) override {
            auto renderer = app.get<CoreState>()->renderer;
            if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0, 1.0f, 1.0f});
            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<MaterialRes>({createUniformBuffer<scenePipeline::PerMaterial>(renderer.get())});
        }

        void regModel(flecs::entity app, flecs::entity e) override {
            auto renderer = app.get<CoreState>()->renderer;

            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});
            if (!e.has<MaterialRes>()) e.add_instanceof(materialPreset);
            if (!e.has<MeshRes>()) e.add_instanceof(meshPreset);
            if (!e.has<DiffuseTexture>()) e.set<DiffuseTexture>({texturePreset});

            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<ModelRes>({createUniformBuffer<scenePipeline::PerObject>(renderer.get())});
        }

        void regPointLight(flecs::entity app, flecs::entity e) override {
            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Intensity>()) e.set<Intensity>({1.0f});
            if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0f, 1.0f, 1.0f});
            if (!e.has<Distance>()) e.set<Distance>({5.0f});
            if (!e.has_instanceof(app)) e.add_instanceof(app);
        }

        void regViewport(flecs::entity app, flecs::entity e) override {
            auto renderer = app.get<CoreState>()->renderer;
            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
            if (!e.has<Extent2D>()) e.set<Extent2D>(*app.get<Extent2D>());

            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<ViewportRes>({createUniformBuffer<scenePipeline::PerViewport>(renderer.get())});
        }

    private:
        flecs::entity meshPreset;
        flecs::entity texturePreset;
        flecs::entity materialPreset;
    };

    void LLGLModule::reg(flecs::entity app) {
        auto ecs = app.world();

        initCoreState(app);
        initGuiState(app);
        initSceneState(app);

        auto texture = ecs.entity().set<Path>({"default.jpg"});
        auto mesh = ecs.entity().set<Path>({"cube.obj"});
        auto material = ecs.entity().set<DiffuseColor>({1.0f, 1.0f, 1.0f});

        Application manager = std::make_shared<LLGLApplication>( mesh, texture, material);
        manager->regTexture(app, texture);
        manager->regMesh(app, mesh);
        manager->regMaterial(app, material);

        app.set<Application>({std::move(manager)});
        app.set<Relative>(Relative{false});
    }
}
