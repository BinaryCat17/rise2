#include "scene.hpp"
#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "resources.hpp"

namespace rise::scene {
  using namespace rise;

	void makeModel(entt::registry& r, entt::entity entity, Mesh mesh, Position position) {
        auto &resources = r.ctx<resources::Instance>();

        r.emplace<Position>(entity, position);

        auto& instance = r.ctx<resources::Instance>();

        glm::mat4 transformMatrix = glm::translate(glm::mat4(1), position);

        auto renderer = r.ctx<LLGL::RenderSystem *>();
        auto buffer = resources::createUniformBuffer(renderer, transformMatrix);

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = resources.pipeline.layout;
        resourceHeapDesc.resourceViews.emplace_back(resources.camera);
        resourceHeapDesc.resourceViews.emplace_back(buffer);
        auto heap = renderer->CreateResourceHeap(resourceHeapDesc);

        r.emplace<MeshDataId>(entity,
                resources.meshData.size() - 1,
                MeshDataBinding);
    }


    void renderScene(LLGL::CommandBuffer *cmd, entt::registry &r) {
        auto &resources = r.ctx<SceneResources>();

        r.view<MeshDataId, MeshId>().
                each([cmd, &resources](entt::entity, MeshDataId dataId, MeshId meshId) {
            cmd->SetResourceHeap(*resources.heaps[dataId.index]);
            cmd->SetVertexBuffer(*resources.meshes[meshId.index].vertices);
            cmd->SetIndexBuffer(*resources.meshes[meshId.index].indices);
            cmd->DrawIndexed(resources.meshes[meshId.index].numIndices, 0);
        });
    }
}