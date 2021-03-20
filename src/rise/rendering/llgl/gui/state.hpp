#pragma once
#include <flecs.h>
#include <LLGL/LLGL.h>
#include "../scene/mesh.hpp"

namespace rise::rendering {
    struct GuiState {
        LLGL::PipelineLayout* layout = nullptr;
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::VertexFormat format;
        LLGL::ResourceHeap* heap = nullptr;
        LLGL::Buffer* uniform = nullptr;
        MeshRes guiMesh;
    };

    void importGuiState(flecs::world &ecs);
}