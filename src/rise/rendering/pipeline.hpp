#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace rise {
    struct PipelineData {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::ShaderProgram *program = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
    };

    namespace scenePipeline {
        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 texCoord;
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

        struct Global {
            alignas(16) PointLight pointLights[maxLightCount] = {};
        };

        struct PerCamera {
            alignas(16) glm::mat4 view = {};
            alignas(16) glm::mat4 projection = {};
        };

        struct PerMaterial {
            alignas(16) glm::vec4 diffuseColor = {};
        };

        struct PerObject {
            alignas(16) glm::mat4 transform = {};
        };


        PipelineData make(LLGL::RenderSystem* renderer, std::string const& root);
    };


    namespace guiPipeline {
        struct Global {
            alignas(8) glm::vec2 scale;
            alignas(8) glm::vec2 translate;
        };

        PipelineData make(LLGL::RenderSystem* renderer, std::string const& root);
    }
}

namespace std {
    template<>
    struct hash<rise::scenePipeline::Vertex> {
        size_t operator()(rise::scenePipeline::Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                    (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
