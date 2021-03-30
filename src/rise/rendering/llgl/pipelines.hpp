#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace rise::rendering::scenePipeline {
    struct Vertex {
        glm::vec3 pos{};
        glm::vec3 normal{};
        glm::vec2 texCoord{};
    };

    inline bool operator==(const Vertex &lhs, const Vertex &rhs) {
        return lhs.pos == rhs.pos &&
                lhs.normal == rhs.normal &&
                lhs.texCoord == rhs.texCoord;
    }

    inline bool operator!=(const Vertex &lhs, const Vertex &rhs) {
        return !(rhs == lhs);
    }

    struct PointLight {
        alignas(16) glm::vec3 position = {};
        alignas(16) glm::vec3 diffuse = {};
        alignas(4) float distance = -1;
        alignas(4) float intensity = 0;
    };

    static const size_t maxLightCount = 32;

    struct PerViewport {
        alignas(16) glm::mat4 view = {};
        alignas(16) glm::mat4 projection = {};
        alignas(16) PointLight pointLights[maxLightCount] = {};
    };

    struct PerMaterial {
        alignas(16) glm::vec4 diffuseColor = {};
    };

    struct PerObject {
        alignas(16) glm::mat4 transform = {};
    };

    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer);

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program);
}

namespace rise::rendering::guiPipeline {
    struct Global {
        alignas(8) glm::vec2 scale;
        alignas(8) glm::vec2 translate;
    };

    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer);

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program);
}

namespace rise::rendering::shadowPipeline {
    using Vertex = scenePipeline::Vertex;

    const LLGL::Extent2D resolution = {256, 256};

    struct PerObject {
        alignas(16) glm::mat4 transform = {};
    };

    struct PerLightMatrices {
        alignas(16) glm::mat4 lightSpaceMatrix[6];
    };

    struct PerLightParameters {
        alignas(16) glm::vec3 lightPos;
        alignas(4) float farPlane;
    };

    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer);

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program,
            LLGL::RenderPass const *pass);
}

namespace std {
    template<>
    struct hash<rise::rendering::scenePipeline::Vertex> {
        size_t operator()(rise::rendering::scenePipeline::Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                    (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
