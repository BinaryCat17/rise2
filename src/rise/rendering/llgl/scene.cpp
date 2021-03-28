#include "scene.hpp"
#include "utils.hpp"

namespace rise::rendering {
    void regSceneState(flecs::entity e) {
        auto ecs = e.world();
    }

    void initSceneState(flecs::entity e, ApplicationState &state, Path const &path) {
        auto const &core = state.core;
        auto const &root = path.file;
        auto &scene = state.scene;
        auto &presets = state.presets;

        scene.format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        scene.layout = scenePipeline::createLayout(core.renderer.get());
        auto program = createShaderProgram(core.renderer.get(),
                root + "/shaders/scene", scene.format);
        scene.pipeline = scenePipeline::createPipeline(core.renderer.get(), scene.layout, program);

        auto ecs = e.world();
        presets.material = ecs.entity().set<RegTo>({e}).
                set<DiffuseColor>({1.0f, 1.0f, 1.0f}).
                add<Material>();

        presets.mesh = ecs.entity().set<RegTo>({e}).
                set<Path>({"cube.obj"}).
                add<Mesh>();

        presets.texture = ecs.entity().set<RegTo>({e}).
                set<Path>({"default.jpg"}).
                add<Texture>();
    };

    void renderScene(flecs::entity, ApplicationRef applicationRef, ViewportRef viewportRef,
            MeshId meshId, ModelId modelId) {
        auto cmdBuf = applicationRef.ref->id->core.cmdBuf;
        cmdBuf->SetPipelineState(*applicationRef.ref->id->scene.pipeline);

        auto position = *getOrDefault(viewportRef.ref.entity(), Position2D{0, 0});
        auto size = *getOrDefault(viewportRef.ref.entity(),
                *applicationRef.ref.entity().get<Extent2D>());
        LLGL::Viewport viewport{position.x, position.y, size.width, size.height};
        cmdBuf->SetViewport(viewport);

        auto &manager = applicationRef.ref->id->manager;
        auto const &model = std::get<eModelState>(manager.model.states.at(modelId.id)).get();
        auto const &mesh = std::get<eMeshState>(manager.mesh.states.at(meshId.id)).get();

        cmdBuf->SetResourceHeap(*model.heap);
        cmdBuf->SetVertexBuffer(*mesh.vertices);
        cmdBuf->SetIndexBuffer(*mesh.indices);
        cmdBuf->DrawIndexed(mesh.numIndices, 0);
    }

    void importSceneState(flecs::world &ecs) {
        ecs.system<>("regSceneState", "Application").kind(flecs::OnAdd).each(regSceneState);
    }
}
