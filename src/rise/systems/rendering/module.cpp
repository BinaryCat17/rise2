#include "module.hpp"
#include "core/systems.hpp"
#include "scene/systems.hpp"
#include "scene/pipeline.hpp"
#include "gui/systems.hpp"
#include "gui/pipeline.hpp"
#include "core/utils.hpp"

namespace rise::systems {
    using namespace rendering;

    struct PrefabsT {
        flecs::type mesh;
        flecs::type texture;
        flecs::type material;
        flecs::type model;
        flecs::type pointLight;
        flecs::type viewport;
        flecs::type application;
    };
    using Prefabs = std::shared_ptr<PrefabsT>;

    struct RenderSystems {
        flecs::entity gui;
        flecs::entity scene;
    };

    Rendering::Rendering(flecs::world &ecs) {
        // components
        ecs.module<Rendering>("rise.systems.rendering");
        ecs.component<RenderSystem>("RenderSystem");
        ecs.component<Window>("Window");
        ecs.component<Context>("Context");
        ecs.component<CommandBuffer>("Queue");
        ecs.component<Sampler>("Sampler");
        ecs.component<ResourceHeap>("ResourceHeap");
        ecs.component<Texture>("Texture");
        ecs.component<PipelineLayout>("PipelineLayout");
        ecs.component<Pipeline>("Pipeline");
        ecs.component<VertexFormat>("VertexFormat");
        ecs.component<Mesh>("MeshRes");
        ecs.component<CoreTag>("CoreTag");
        ecs.component<GuiParameters>("GuiParameters");
        ecs.component<GuiTag>("GuiTag");
        ecs.component<DiffuseTextureRes>("DiffuseTextureRes");
        ecs.component<MaterialRes>("MaterialRes");
        ecs.component<ViewportRes>("ViewportRes");
        ecs.component<TransformRes>("TransformRes");
        ecs.component<SceneTag>("SceneTag");

        // observers
        ecs.system<RenderSystem, const VertexFormat, Mesh, const Path>("updateMesh").
                kind(flecs::OnSet).each(updateMesh);

        ecs.system<RenderSystem, Texture, const Path>("updateTexture").kind(flecs::OnSet).
                each(updateTexture);

        ecs.system<Window, Context, const Extent2D>("updateWindowSize").kind(flecs::OnSet).
                each(updateWindowSize);

        ecs.system<RenderSystem, const PipelineLayout, ResourceHeap, const DiffuseTextureRes,
                const MaterialRes, const TransformRes, const ViewportRes, const Sampler>(
                "updateResourceHeap").kind(flecs::OnSet).each(updateResourceHeap);

        ecs.system<RenderSystem, const TransformRes, const Position3D, const Rotation3D,
                const Scale3D>("updateTransform").kind(flecs::OnSet).each(updateTransform);

        ecs.system<RenderSystem, const MaterialRes, const DiffuseColor>("updateMaterial").
                kind(flecs::OnSet).each(updateMaterial);

        ecs.system<RenderSystem, ViewportRes, const Extent2D, const Position3D,
                const Rotation3D>("updateViewport").kind(flecs::OnSet).each(updateViewport);

        // pre store

        ecs.system<GuiContext, RenderSystem, const GuiParameters, Mesh, const VertexFormat>(
                "updateGuiResources", "GuiTag").kind(flecs::PreStore).each(updateResources);

        ecs.system<CommandBuffer, Context>("prepareRender", "CoreTag").kind(flecs::PreStore).
                each(prepareRender);

        ecs.system<RenderSystem, CommandBuffer, Pipeline, Extent2D>("renderScene", "SceneTag").
                kind(flecs::PreStore).each(renderScene);

        ecs.system<GuiContext, Window>("prepareImgui", "GuiTag").kind(flecs::PreStore).
                each(prepareImgui);

        // on store

        ecs.system<CommandBuffer, Queue, Context>("submitRender", "CoreTag").kind(flecs::OnStore).
                each(submitRender);
        ecs.system<GuiContext>("processImGui", "GuiTag").kind(flecs::OnStore).each(processImGui);
        ecs.system<GuiContext, const Pipeline, const Extent2D, RenderSystem, const CommandBuffer,
                const Mesh, const ResourceHeap>("renderGui", "GuiTag").kind(flecs::OnStore).
                each(renderGui);
    }

    void Rendering::regApplication(flecs::entity e) {
        auto ecs = e.world();

        auto applicationPrefab = ecs.prefab("ApplicationPrefab").
                set<Extent2D>({800.0f, 600.0f}).
                set<Path>({"./"});

        e.add_instanceof(applicationPrefab);
        e.add<CoreTag>();

        initCoreState(e);

        RenderSystems systems{
                ecs.entity().add_instanceof(e).add<GuiTag>(),
                ecs.entity().add_instanceof(e).add<SceneTag>()
        };
        initGuiPipeline(systems.gui);
        initGui(systems.gui);

        initScenePipeline(systems.scene);

        e.set<RenderSystems>(systems);

        auto meshPrefab = ecs.prefab("MeshPrefab").
                set<Path>({"cube.obj"});
        regMesh(e, meshPrefab);
        auto texturePrefab = ecs.prefab("TexturePrefab").
                set<Path>({"paper.jpg"});
        regTexture(e, texturePrefab);
        auto materialPrefab = ecs.prefab("MaterialPrefab").
                set<DiffuseColor>({1.0f, 1.0f, 1.0f});
        regMaterial(e, materialPrefab);
        auto modelPrefab = ecs.prefab("ModelPrefab").
                set<Position3D>({0.0f, 0.0f, 0.0f}).
                set<Rotation3D>({0.0f, 0.0f, 0.0f}).
                set<Scale3D>({1.0f, 1.0f, 1.0f});
        auto pointLightPrefab = ecs.prefab("PointLightPrefab").
                set<Position3D>({0.0f, 0.0f, 0.0f}).
                set<Intensity>({1.f}).
                set<DiffuseColor>({1.0f, 1.0f, 1.0f}).
                set<Distance>({5.f});
        auto viewportPrefab = ecs.prefab("ViewportPrefab").
                set<Position3D>({0.0f, 0.0f, 0.0f}).
                set<Rotation3D>({0.0f, 0.0f, 0.0f}).
                set<Position2D>({0.0f, 0.0f}).
                set<Extent2D>({800.0f, 600.0f});

        auto prefabs = std::make_shared<PrefabsT>(PrefabsT{
                ecs.type("MeshBase").add_instanceof(meshPrefab).add<Path>(),
                ecs.type("TextureBase").add_instanceof(texturePrefab).add<Path>(),
                ecs.type("MaterialBase").add_instanceof(materialPrefab).add<DiffuseColor>(),
                ecs.type("ModelBase").add_instanceof(modelPrefab).
                        add<Position3D, Rotation3D, Scale3D>(),
                ecs.type("PointLightBase").add_instanceof(pointLightPrefab).
                        add<Position3D>().add<Intensity>().add<DiffuseColor>().add<Distance>(),
                ecs.type("ViewportBase").add_instanceof(viewportPrefab).
                        add<Position3D>().add<Rotation3D>().add<Position2D>().add<Extent2D>(),
                ecs.type("ApplicationBase").add_instanceof(applicationPrefab).
                        add<Extent2D>().add<Path>()
        });

        prefabs->model.add(prefabs->mesh).add(prefabs->texture);
        e.set<Prefabs>(prefabs);
    }

    void Rendering::regMesh(flecs::entity app, flecs::entity e) {
        auto& prefabs = checkGet<Prefabs>(app);
        e.add(prefabs->mesh).add_instanceof(app.get<RenderSystems>()->scene);
        e.add<Mesh>();
    }

    void Rendering::regTexture(flecs::entity app, flecs::entity e) {
        auto& prefabs = checkGet<Prefabs>(app);
        e.add(prefabs->texture).add_instanceof(app.get<RenderSystems>()->scene);
        e.add<Texture>();
    }

    void Rendering::regMaterial(flecs::entity app, flecs::entity e) {
        auto& prefabs = checkGet<Prefabs>(app);
        auto renderer = *app.get<RenderSystem>();
        e.add(prefabs->material).add_instanceof(app.get<RenderSystems>()->scene);
        e.set<MaterialRes>({createUniformBuffer<scenePipeline::PerMaterial>(renderer.get())});
    }

    void Rendering::regModel(flecs::entity app, flecs::entity e) {
        auto& prefabs = checkGet<Prefabs>(app);
        auto renderer = *app.get<RenderSystem>();
        e.add(prefabs->model).add_instanceof(app.get<RenderSystems>()->scene);
        e.set<TransformRes>({createUniformBuffer<scenePipeline::PerObject>(renderer.get())});
        e.add<ResourceHeap>();
    }

    void Rendering::regPointLight(flecs::entity app, flecs::entity e) {
        auto& prefabs = checkGet<Prefabs>(app);
        e.add(prefabs->pointLight).add_instanceof(app.get<RenderSystems>()->scene);
    }

    void Rendering::regViewport(flecs::entity app, flecs::entity e) {
        auto renderer = *app.get<RenderSystem>();
        auto& prefabs = checkGet<Prefabs>(app);
        e.add(prefabs->viewport).add_instanceof(app.get<RenderSystems>()->scene);
        e.set<TransformRes>({createUniformBuffer<scenePipeline::PerViewport>(renderer.get())});
    }
}
