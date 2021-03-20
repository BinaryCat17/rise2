#include "systems.hpp"

namespace rise::rendering {
    void renderScene(flecs::entity, CoreState &core, SceneState &scene, Position2D position,
            Extent2D size, MeshRes mesh, ModelRes model) {
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
