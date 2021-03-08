#include "model.hpp"


namespace rise::systems::rendering {
    void updateResourceHeap(RenderState &render, PipelineState &pipeline, ModelResource &model) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = pipeline.layout;
        resourceHeapDesc.resourceViews.emplace_back(model.viewportBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.materialBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.objectBuffer);
        resourceHeapDesc.resourceViews.emplace_back(render.sampler);
        resourceHeapDesc.resourceViews.emplace_back(model.texture);

        if (model.heap) {
            render.renderer->Release(*model.heap);
        }

        model.heap = render.renderer->CreateResourceHeap(resourceHeapDesc);
    }
}