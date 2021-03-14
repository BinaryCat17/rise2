#pragma once

#include <imgui.h>
#include <LLGL/LLGL.h>
#include "../core/resources.hpp"
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    struct GuiState {
        LLGL::PipelineLayout* layout = nullptr;
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::VertexFormat format;
        LLGL::ResourceHeap* heap = nullptr;
        LLGL::Buffer* uniform;
        MeshRes guiMesh;
    };
}
