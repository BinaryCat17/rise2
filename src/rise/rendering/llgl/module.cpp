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
        ecs.import<components::Rendering>();
        ecs.module<Rendering>("rise::systems::rendering");
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
                "[in] ANY:rise.components.rendering.RPosition3D,"
                "[in] ANY:rise.components.rendering.RExtent2D,"
                "[in] ANY:rise.components.rendering.RRotation3D").
                kind(flecs::OnSet).each(dirtyViewportCamera);

        ecs.system<ViewportRes>("dirtyViewportLight",
                "OWNED:ViewportRes,"
                "[in] rise.components.rendering.RPosition3D,"
                "[in] rise.components.rendering.RDiffuseColor,"
                "[in] rise.components.rendering.RIntensity,"
                "[in] rise.components.rendering.RDistance").
                kind(flecs::OnSet).each(dirtyViewportLight);

        auto e = ecs.entity().add<Relative>();
        std::cout << e.type().str() << std::endl;

        ecs.system<CoreState, const Relative>("updateRelative",
                "rise.components.rendering.RRelative").kind(flecs::OnSet).each(updateRelative);

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

    void LLGLModule::reg(flecs::entity app) {
        auto ecs = e.world();

        initCoreState(e);
        initGuiState(e);
        initSceneState(e);

        Defaults defaults;

        defaults.texture = ecs.entity().set<Path>({"default.jpg"});
        regTexture(e, defaults.texture);
        defaults.mesh = ecs.entity().set<Path>({"cube.obj"});
        regMesh(e, defaults.mesh);
        defaults.material = ecs.entity().set<DiffuseColor>({1.0f, 1.0f, 1.0f});
        regMaterial(e, defaults.material);

        e.set<Defaults>(defaults);
        e.set<Relative>(Relative{false});
    }

    struct LLGLApplication : Application {
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
            auto &prefabs = checkGet<Defaults>(app);

            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Scale3D>()) e.set<Scale3D>({1.0f, 1.0f, 1.0f});
            if (!e.has<MaterialRes>()) e.add_instanceof(prefabs.material);
            if (!e.has<MeshRes>()) e.add_instanceof(prefabs.mesh);
            if (!e.has<DiffuseTexture>()) e.set<DiffuseTexture>({prefabs.texture});

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
            if (!e.has<Extent2D>()) e.set<Extent2D>(checkGet<Extent2D>(app));

            if (!e.has_instanceof(app)) e.add_instanceof(app);
            e.set<ViewportRes>({createUniformBuffer<scenePipeline::PerViewport>(renderer.get())});
        }

        flecs::entity mesh;
        flecs::entity texture;
        flecs::entity material;
    };
}
