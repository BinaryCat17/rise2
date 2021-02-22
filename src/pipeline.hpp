#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

namespace rise {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;

        static LLGL::VertexFormat format();
    };

    inline bool operator==(const Vertex &lhs, const Vertex &rhs) {
        return lhs.pos == rhs.pos &&
                lhs.normal == rhs.normal &&
                lhs.texCoord == rhs.texCoord;
    }

    inline bool operator!=(const Vertex &lhs, const Vertex &rhs) {
        return !(rhs == lhs);
    }

    struct PipelineData {
        LLGL::PipelineLayout *layout;
        LLGL::ShaderProgram *program;
        LLGL::PipelineState *data;
    };

    LLGL::PipelineLayout *makeLayout(LLGL::RenderSystem *renderer);

    LLGL::PipelineLayout *makeGuiLayout(LLGL::RenderSystem *renderer);

    LLGL::ShaderProgram *makeProgram(LLGL::RenderSystem *renderer, std::string const &root);

    LLGL::PipelineState *makePipeline(LLGL::RenderSystem *renderer, LLGL::PipelineLayout *layout,
            LLGL::ShaderProgram *program);
}

namespace std {
    template<>
    struct hash<rise::Vertex> {
        size_t operator()(rise::Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                    (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
