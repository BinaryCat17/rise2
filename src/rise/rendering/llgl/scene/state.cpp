#include "state.hpp"
#include "../core/state.hpp"
#include "../core/utils.hpp"
#include "../module.hpp"
#include "pipeline.hpp"

namespace rise::rendering {
    void regSceneState(flecs::entity e) {
        if(e.has<LLGLApplication>()) {
            auto ecs = e.world();
            e.set<SceneState>({});
            e.set<Presets>({ecs.entity(), ecs.entity(), ecs.entity()});
        }
    }

    void initSceneState(flecs::entity e, CoreState &core, SceneState &scene, Presets &presets) {
        scene.format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        scene.layout = scenePipeline::createLayout(core.renderer.get());
        auto program = createShaderProgram(core.renderer.get(),
                core.root + "/shaders/scene", scene.format);
        scene.pipeline = scenePipeline::createPipeline(core.renderer.get(), scene.layout, program);

        presets.material.set<RegTo>({e}).
                set<DiffuseColor>({1.0f, 1.0f, 1.0f}).
                add<Material>();

        presets.mesh.set<RegTo>({e}).
                set<Path>({"cube.obj"}).
                add<Mesh>();

        presets.texture.set<RegTo>({e}).
                set<Path>({"default.jpg"}).
                add<Texture>();
    };

    void importSceneState(flecs::world &ecs) {
        ecs.system<>("regSceneState", "Application").kind(flecs::OnAdd).each(regSceneState);

        ecs.system<CoreState, SceneState, Presets>("initSceneState", "Application").
                kind(flecs::OnSet).each(initSceneState);
    }
}
