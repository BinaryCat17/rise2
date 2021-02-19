#pragma once
#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

namespace rise::shader {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;

        static LLGL::VertexFormat format();
    };

    LLGL::PipelineLayout *layout(LLGL::RenderSystem *renderer);

    LLGL::ShaderProgram* program(LLGL::RenderSystem *renderer, std::string const& root);
}