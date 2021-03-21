#include "systems.hpp"

namespace rise::rendering {
    void renderScene(flecs::entity, RegTo state, RenderTo target, MeshRes mesh, ModelRes model) {
        auto &core = *state.e.get<CoreState>();
        auto &scene = *state.e.get<SceneState>();
        auto position = *target.e.get<Position2D>();
        auto size = *target.e.get<Extent2D>();

        auto cmdBuf = core.cmdBuf;
        cmdBuf->SetPipelineState(*scene.pipeline);

        LLGL::Viewport viewport{position.x, position.y, size.width, size.height};
        cmdBuf->SetViewport(viewport);

        cmdBuf->SetResourceHeap(*model.heap);
        cmdBuf->SetVertexBuffer(*mesh.vertices);
        cmdBuf->SetIndexBuffer(*mesh.indices);
        cmdBuf->DrawIndexed(mesh.numIndices, 0);
    }
}
