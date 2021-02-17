#include "rendering.hpp"
#include <iostream>

namespace rise {
	std::unique_ptr<LLGL::RenderSystem> createRenderer() {
		return LLGL::RenderSystem::Load("OpenGL");
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
		window.SetTitle(L"LLGL Example: Hello Triangle");
		window.Show();

		return Context{
			&window,
			context
		};
	}

	VertexInput createVertexInput(LLGL::RenderSystem* renderer) {
		struct Vertex {
			float position[3];
			uint8_t color[4];
		};

		Vertex vertices[3] = {
						Vertex{{0.0f, 0.5f, 0.0f},
									 {255,  0,    0, 255}},
						Vertex{{0.5f, -0.5f, 0.0f},
									 {0,    255,   0, 255}},
						Vertex{{-0.5f, -0.5f, 0.0f},
									 {0,     0,     255, 255}},
		};

		LLGL::VertexFormat vertexFormat;
		vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
		vertexFormat.AppendAttribute({ "color", LLGL::Format::RGBA8UNorm });

		LLGL::BufferDescriptor myVBufferDesc;
		myVBufferDesc.size = sizeof(vertices);            // Size (in bytes) of the buffer
		myVBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer; // Use for vertex buffer binding
		myVBufferDesc.vertexAttribs = vertexFormat.attributes;     // Vertex buffer attributes
		return { renderer->CreateBuffer(myVBufferDesc, vertices), std::move(vertexFormat) };
	}

	UniformData createUniformData(LLGL::RenderSystem* renderer) {
		LLGL::BufferDescriptor uniformBufferDesc;
		uniformBufferDesc.size = sizeof(glm::mat4);
		uniformBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer; // Use for vertex buffer binding
		uniformBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
		LLGL::Buffer* uniformBuffer = renderer->CreateBuffer(uniformBufferDesc);
		void* pData = renderer->MapBuffer(*uniformBuffer, LLGL::CPUAccess::ReadWrite);
		return UniformData{
			uniformBuffer,
			reinterpret_cast<glm::mat4*>(pData)
		};
	}

	LLGL::ShaderProgram* createShaderProgram(LLGL::RenderSystem* renderer, std::string const& root,
		LLGL::VertexFormat const& format) {
		std::string vertPath = root + "/shaders/shader.vert";
		std::string fragPath = root + "/shaders/shader.frag";

		LLGL::ShaderDescriptor VSDesc, FSDesc;
		VSDesc = { LLGL::ShaderType::Vertex, vertPath.data() };
		FSDesc = { LLGL::ShaderType::Fragment, fragPath.data() };

		VSDesc.vertex.inputAttribs = format.attributes;

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

		return shaderProgram;
	}

	std::pair<LLGL::PipelineState*, LLGL::PipelineLayout*> createPipeline(
		LLGL::RenderSystem* renderer, LLGL::ShaderProgram* program) {
		LLGL::PipelineLayoutDescriptor layoutDesc;
		layoutDesc.bindings = {
						LLGL::BindingDescriptor{LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
										LLGL::StageFlags::VertexStage, 0},
		};
		auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

		LLGL::GraphicsPipelineDescriptor pipelineDesc;
		pipelineDesc.shaderProgram = program;
		pipelineDesc.pipelineLayout = pipelineLayout;
		pipelineDesc.rasterizer.multiSampleEnabled = true;
		return { renderer->CreatePipelineState(pipelineDesc), pipelineLayout };
	}

	LLGL::ResourceHeap* createResources(LLGL::RenderSystem* renderer,
		LLGL::PipelineLayout* layout, unsigned width, unsigned height) {
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height,
			0.1f, 100.0f);

		glm::mat4 View = glm::lookAt(
			glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		glm::mat4 Model = glm::mat4(1.0f);

		glm::mat4 mvp = Projection * View * Model;

		LLGL::BufferDescriptor uniformBufferDesc;
		uniformBufferDesc.size = sizeof(glm::mat4);
		uniformBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer; // Use for vertex buffer binding
		LLGL::Buffer* uniformBuffer = renderer->CreateBuffer(uniformBufferDesc, &mvp);

		LLGL::ResourceHeapDescriptor resourceHeapDesc;
		resourceHeapDesc.pipelineLayout = layout;
		resourceHeapDesc.resourceViews.emplace_back(uniformBuffer);
		return renderer->CreateResourceHeap(resourceHeapDesc);
	}
}