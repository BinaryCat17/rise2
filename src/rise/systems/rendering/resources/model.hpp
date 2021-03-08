#pragma once
#include <LLGL/LLGL.h>
#include "systems/rendering/rendering/guiSystem.hpp"

namespace rise::systems::rendering {
    struct ModelResource {
        LLGL::Buffer *viewportBuffer = nullptr;
        LLGL::Buffer *materialBuffer = nullptr;
        LLGL::Buffer *objectBuffer = nullptr;
        LLGL::Texture *texture = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
        bool dirtyHeap = true;
        bool dirtyModel = true;
    };



    void updateResourceHeap(RenderState& render, PipelineState& pipeline, ModelResource& model);
}
