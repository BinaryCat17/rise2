#pragma once
#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

namespace rise {
    std::unique_ptr<LLGL::RenderSystem> createRenderer();

    struct Context {
      LLGL::Window* window;
      LLGL::RenderContext* context;
    };

    Context createContext(LLGL::RenderSystem *renderer, unsigned width, unsigned height);

    struct VertexInput {
      LLGL::Buffer* buffer;
      LLGL::VertexFormat format;
    };

    VertexInput createVertexInput(LLGL::RenderSystem *renderer);

    struct UniformData {
      LLGL::Buffer* uniformBuffer;
      glm::mat4* mvp;
    };

    UniformData createUniformData(LLGL::RenderSystem *renderer);

    struct ShaderResources {
      VertexInput vertex;
      UniformData uniform;
    };

    ShaderResources createShaderResources(LLGL::RenderSystem *renderer);

    struct Pipeline {
      LLGL::ResourceHeap* resourcesHeap;
      LLGL::ShaderProgram* shader;
      LLGL::PipelineState* state;
      LLGL::PipelineLayout* layout;
    };

    Pipeline createPipeline(LLGL::RenderSystem *renderer, ShaderResources const& resources);
}