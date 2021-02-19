#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace rise {
	namespace impl {
		struct MeshRes {
			LLGL::VertexFormat format = {};
			LLGL::Buffer* vertices = nullptr;
			LLGL::Buffer* indices = nullptr;
			unsigned numIndices = 0;
		};

		struct ModelData {
			LLGL::Buffer* uniformBuffer;
			LLGL::ResourceHeap* heap;
		};

		struct Resources {
			std::vector<MeshRes> meshes;
			std::vector<ModelData> models;
		};
	}

	struct Mesh {
		size_t id;
	};

	struct Position : glm::vec3 {};

	struct Instance {
		std::unique_ptr<LLGL::RenderSystem> renderer;
		LLGL::RenderContext* context;
		LLGL::Window* window;
		LLGL::PipelineState* pipeline;
		LLGL::PipelineLayout* layout;
		LLGL::ShaderProgram* program;
		impl::Resources resources;
	};

	Instance makeInstance(std::string const& root, unsigned width, unsigned height);

	void init(entt::registry& r, Instance* instance);

	Mesh makeMesh(entt::registry& r, std::string const& path);

	void renderLoop(entt::registry& r);






































	std::unique_ptr<LLGL::RenderSystem> createRenderer();

	struct Context {
		LLGL::Window* window = nullptr;
		LLGL::RenderContext* context = nullptr;
	};

	Context createContext(LLGL::RenderSystem* renderer, unsigned width, unsigned height);

	struct ShaderResources {
		LLGL::PipelineLayout* pipelineLayout = nullptr;
		LLGL::ResourceHeap* resourcesHeap = nullptr;
	};

	ShaderResources createShaderResources(LLGL::RenderSystem* renderer);

	void bindResources(LLGL::CommandBuffer* cmdBuf, ShaderResources& resources);

	struct Pipeline {
		LLGL::ShaderProgram* shaders = nullptr;
		LLGL::PipelineState* state = nullptr;
		LLGL::PipelineLayout* layout = nullptr;
	};

	Pipeline createPipeline(LLGL::RenderSystem* renderer, std::string const& root,
		ShaderResources const& resources);

	void bindPipeline(LLGL::CommandBuffer* cmdBuf, Pipeline& pipeline);

	template<typename FnT>
	void renderLoop(LLGL::RenderSystem* renderer, Context context, FnT&& f) {
		LLGL::CommandQueue* cmdQueue = renderer->GetCommandQueue();
		LLGL::CommandBuffer* cmdBuffer = renderer->CreateCommandBuffer();

		while (context.window->ProcessEvents()) {
			cmdBuffer->Begin();
			cmdBuffer->BeginRenderPass(*context.context);
			cmdBuffer->SetViewport(context.context->GetResolution());
			cmdBuffer->Clear(LLGL::ClearFlags::Color);
			f(cmdBuffer);
			cmdBuffer->EndRenderPass();
			cmdBuffer->End();
			cmdQueue->Submit(*cmdBuffer);
			context.context->Present();
		}
	}
}