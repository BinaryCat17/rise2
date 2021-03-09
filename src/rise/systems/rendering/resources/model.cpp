#include "model.hpp"


namespace rise::systems::rendering {
    void updateModelSystem(flecs::entity, ApplicationResource& app, ModelResource& model) {
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