#include "model.hpp"
#include "components/rendering/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateModelHeapSystem(flecs::entity, ApplicationResource& app, ModelResource& model) {
        if(model.dirtyHeap) {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            resourceHeapDesc.pipelineLayout = app.scene.pipeline.layout;
            resourceHeapDesc.resourceViews.emplace_back(model.viewportBuffer);
            resourceHeapDesc.resourceViews.emplace_back(model.materialBuffer);
            resourceHeapDesc.resourceViews.emplace_back(model.objectBuffer);
            resourceHeapDesc.resourceViews.emplace_back(app.state.sampler);
            resourceHeapDesc.resourceViews.emplace_back(model.texture);

            if (model.heap) {
                app.state.renderer->Release(*model.heap);
            }

            model.heap = app.state.renderer->CreateResourceHeap(resourceHeapDesc);
        }
    }

    void updateModelTransformSystem(flecs::entity, ApplicationResource& app, ModelResource& model) {
        if(model.dirtyModel) {
            glm::mat4 mat = glm::translate(glm::mat4(1), position.val);
            float angle = std::max({rotation.val.x, rotation.val.y, rotation.val.z});
            mat = glm::rotate(mat, glm::radians(angle), glm::normalize(rotation.val));
            mat = glm::scale(mat, scale.val);

            updateUniformBuffer(instance.renderer.get(), res.objectBuffer,
                    scenePipeline::PerObject{mat});
            res.dirtyModel = false;
        }
    }
}