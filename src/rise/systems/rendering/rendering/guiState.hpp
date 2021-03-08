#pragma once
#include "state.hpp"
#include "systems/rendering/pipeline/state.hpp"

namespace rise::systems::rendering {
    struct GuiState {
        PipelineState pipeline;
        LLGL::Buffer *globalData = nullptr;
        LLGL::Texture *fontTexture = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
        LLGL::Sampler *sampler = nullptr;
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned vertexCount = 0;
        unsigned indexCount = 0;
        std::vector<std::function<void()>> callbacks;
    };

    GuiState createGuiState(RenderState &render, std::string const &path);
}