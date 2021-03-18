#pragma once

#include <imgui.h>
#include <LLGL/LLGL.h>
#include "../core/resources.hpp"

namespace rise::rendering {
    struct GuiState {
        LLGL::PipelineLayout* layout = nullptr;
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::VertexFormat format;
        LLGL::ResourceHeap* heap = nullptr;
        LLGL::Buffer* uniform = nullptr;
        MeshRes guiMesh;
    };
}
