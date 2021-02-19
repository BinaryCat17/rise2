#include "rendering.hpp"
#include <iostream>
#include "mesh.hpp"
#include "pipeline.hpp"
#include "resources.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
	LLGL::RenderContext* makeContext(LLGL::RenderSystem* renderer, unsigned width, unsigned height) {
		LLGL::RenderContextDescriptor contextDesc;
		contextDesc.videoMode.resolution = { width, height };
		contextDesc.videoMode.fullscreen = false;
		contextDesc.vsync.enabled = true;
		contextDesc.samples = 8;
		LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc);

		const auto& info = renderer->GetRendererInfo();

		std::cout << "Renderer:         " << info.rendererName << std::endl;
		std::cout << "Device:           " << info.deviceName << std::endl;
		std::cout << "Vendor:           " << info.vendorName << std::endl;
		std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

		return context;
	}

	class Debugger : public LLGL::RenderingDebugger {
	public:
	protected:
		void OnError(LLGL::ErrorType type, Message& message) override {
			std::cerr << "error: " << message.GetText() << std::endl;
		}

		void OnWarning(LLGL::WarningType type, Message& message) override {
			std::cerr << "warning: " << message.GetText() << std::endl;
		}
	};

	std::unique_ptr<LLGL::RenderSystem> createRenderer() {
		static Debugger debugger;
		return LLGL::RenderSystem::Load("OpenGL", nullptr, &debugger);
	}

	Instance makeInstance(std::string const& root, unsigned width, unsigned height) {
		Instance instance;

		instance.renderer = createRenderer();
		instance.context = makeContext(instance.renderer.get(), width, height);

		auto& window = LLGL::CastTo<LLGL::Window>(instance.context->GetSurface());
		window.SetTitle(L"Rise!");
		window.Show();

		instance.window = &window;

		instance.layout = makeLayout(instance.renderer.get());
		instance.program = makeProgram(instance.renderer.get(), root + "/shaders");
		instance.pipeline = makePipeline(instance.renderer.get(), instance.layout, instance.program);

		return instance;
	}

	void reCalc(entt::registry& r, entt::entity e) {
		glm::mat4 mat(1);
		auto& pos = r.get<Position>(e);
		mat = glm::translate(mat, pos);

		auto instance = r.ctx<Instance*>();
		if (auto model = r.try_get<ModelData>(e)) {
			updateUniformBuffer(instance->renderer.get(), model->uniformBuffer, mat);
		}
	}

	void init(entt::registry& r, Instance* instance) {
		r.set<Instance*>(instance);
		r.on_construct<Position>().connect<&reCalc>();
		r.on_update<Position>().connect<&reCalc>();
		
		r.on_construct<Mesh>()
	}






































	class Debugger : public LLGL::RenderingDebugger {
	public:
	protected:
		void OnError(LLGL::ErrorType type, Message& message) override {
			std::cerr << "error: " << message.GetText() << std::endl;
		}

		void OnWarning(LLGL::WarningType type, Message& message) override {
			std::cerr << "warning: " << message.GetText() << std::endl;
		}
	};

	std::unique_ptr<LLGL::RenderSystem> createRenderer() {
		static Debugger debugger;
		return LLGL::RenderSystem::Load("OpenGL", nullptr, &debugger);
	}

	Context createContext(LLGL::RenderSystem* renderer, unsigned width, unsigned height) {
		LLGL::RenderContextDescriptor contextDesc;
		contextDesc.videoMode.resolution = { width, height };
		contextDesc.videoMode.fullscreen = false;
		contextDesc.vsync.enabled = true;
		contextDesc.samples = 8;
		LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc);

		const auto& info = renderer->GetRendererInfo();

		std::cout << "Renderer:         " << info.rendererName << std::endl;
		std::cout << "Device:           " << info.deviceName << std::endl;
		std::cout << "Vendor:           " << info.vendorName << std::endl;
		std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

		auto& window = LLGL::CastTo<LLGL::Window>(context->GetSurface());
		window.SetTitle(L"Rise!");
		window.Show();

		return Context{ &window, context };
	}

	Pipeline createPipeline(LLGL::RenderSystem* renderer, std::string const& root,
		ShaderResources const& resources) {
		std::string vertPath = root + "/shaders/shader.vert";
		std::string fragPath = root + "/shaders/shader.frag";

		LLGL::ShaderDescriptor VSDesc, FSDesc;
		VSDesc = { LLGL::ShaderType::Vertex, vertPath.data() };
		FSDesc = { LLGL::ShaderType::Fragment, fragPath.data() };

		VSDesc.vertex.inputAttribs = getVertexFormat().attributes;

		LLGL::ShaderProgramDescriptor programDesc;
		programDesc.vertexShader = renderer->CreateShader(VSDesc);
		programDesc.fragmentShader = renderer->CreateShader(FSDesc);

		for (auto shader : { programDesc.vertexShader, programDesc.fragmentShader }) {
			std::string log = shader->GetReport();
			if (!log.empty()) {
				std::cerr << log << std::endl;
			}
		}

		auto shaderProgram = renderer->CreateShaderProgram(programDesc);

		if (shaderProgram->HasErrors()) {
			throw std::runtime_error(shaderProgram->GetReport());
		}

		LLGL::GraphicsPipelineDescriptor pipelineDesc;
		pipelineDesc.shaderProgram = shaderProgram;
		pipelineDesc.pipelineLayout = resources.pipelineLayout;
		pipelineDesc.rasterizer.multiSampleEnabled = true;

		auto pipeline = renderer->CreatePipelineState(pipelineDesc);

		return Pipeline{ shaderProgram, pipeline };
	}
}