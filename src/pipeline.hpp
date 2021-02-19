#pragma once
#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

namespace rise {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static LLGL::VertexFormat format();
	};

	struct PipelineData {
		LLGL::PipelineLayout* layout;
		LLGL::ShaderProgram* program;
		LLGL::PipelineState* data;
	};

	LLGL::PipelineLayout* makeLayout(LLGL::RenderSystem* renderer);

	LLGL::ShaderProgram* makeProgram(LLGL::RenderSystem* renderer, std::string const& root);

	LLGL::PipelineState* makePipeline(LLGL::RenderSystem* renderer, LLGL::PipelineLayout* layout,
      LLGL::ShaderProgram* program);

	PipelineData
}