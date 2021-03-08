#pragma once
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct PipelineState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::ShaderProgram *program = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
    };

    LLGL::ShaderProgram* createShaderProgram(LLGL::RenderSystem* renderer, std::string const& root,
            LLGL::VertexFormat const& format);
}