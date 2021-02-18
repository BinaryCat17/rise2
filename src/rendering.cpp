#include "rendering.hpp"
#include <iostream>

namespace rise {
    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            std::cerr << "error: " << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            std::cerr << "warning: " << message.GetText() << std::endl;
        }
    };

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        return LLGL::RenderSystem::Load("OpenGL", nullptr, &debugger);
    }

    Context createContext(LLGL::RenderSystem *renderer, unsigned width, unsigned height) {
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = {width, height};
        contextDesc.videoMode.fullscreen = false;
        contextDesc.vsync.enabled = true;
        contextDesc.samples = 8;
        LLGL::RenderContext *context = renderer->CreateRenderContext(contextDesc);

        const auto &info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        auto &window = LLGL::CastTo<LLGL::Window>(context->GetSurface());
        window.SetTitle(L"Rise!");
        window.Show();

        return Context{ &window, context };
    }

    VertexInput createVertexInput(LLGL::RenderSystem *renderer) {
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({"position", LLGL::Format::RGB32Float});

        LLGL::BufferDescriptor VBufferDesc;
        VBufferDesc.size = sizeof(cube);            // Size (in bytes) of the buffer
        VBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer; // Use for vertex buffer binding
        VBufferDesc.vertexAttribs = vertexFormat.attributes;     // Vertex buffer attributes
        return {renderer->CreateBuffer(VBufferDesc, cube), std::move(vertexFormat)};
    }

    void drawVertices(LLGL::CommandBuffer *cmdBuf, VertexInput &vertexInput) {
        cmdBuf->SetVertexBuffer(*vertexInput.buffer);
        cmdBuf->Draw(12 * 3, 0);
    }

    GlobalShaderData createUniformData(LLGL::RenderSystem *renderer) {
        LLGL::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.size = sizeof(glm::mat4);
        uniformBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer; // Use for vertex buffer binding
        uniformBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
        uniformBufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
        LLGL::Buffer *uniformBuffer = renderer->CreateBuffer(uniformBufferDesc);
        return UniformData{uniformBuffer};
    }

    void updateUniformData(LLGL::RenderSystem *renderer, GlobalShaderData &uniformData,
            Camera const &camera) {
        glm::mat4 model(1);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                (float) camera.width / (float) camera.height, 0.1f, 100.0f);

        glm::mat4 view = glm::lookAt(
                camera.position,
                camera.origin,
                glm::vec3(0, 1, 0)
        );
        void *pData = renderer->MapBuffer(*uniformData.buffer, LLGL::CPUAccess::ReadWrite);
        auto mvp = reinterpret_cast<glm::mat4 *>(pData);
        *mvp = projection * view * model;
        renderer->UnmapBuffer(*uniformData.buffer);
    }

    ShaderResources createShaderResources(LLGL::RenderSystem *renderer) {
        VertexInput vertexInput = createVertexInput(renderer);
        GlobalShaderData uniformData = createUniformData(renderer);

        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage, 0},
        };
        auto layout = renderer->CreatePipelineLayout(layoutDesc);

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = layout;
        resourceHeapDesc.resourceViews.emplace_back(uniformData.buffer);
        auto heap = renderer->CreateResourceHeap(resourceHeapDesc);

        return ShaderResources{
                layout,
                heap,
                vertexInput,
                uniformData,
        };
    }

    void bindResources(LLGL::CommandBuffer *cmdBuf, ShaderResources &resources) {
        cmdBuf->SetResourceHeap(*resources.resourcesHeap);
    }

    Pipeline createPipeline(LLGL::RenderSystem *renderer, std::string const &root,
            ShaderResources const &resources) {
        std::string vertPath = root + "/shaders/shader.vert";
        std::string fragPath = root + "/shaders/shader.frag";

        LLGL::ShaderDescriptor VSDesc, FSDesc;
        VSDesc = {LLGL::ShaderType::Vertex, vertPath.data()};
        FSDesc = {LLGL::ShaderType::Fragment, fragPath.data()};

        VSDesc.vertex.inputAttribs = resources.vertex.format.attributes;

        LLGL::ShaderProgramDescriptor programDesc;
        programDesc.vertexShader = renderer->CreateShader(VSDesc);
        programDesc.fragmentShader = renderer->CreateShader(FSDesc);

        for (auto shader : {programDesc.vertexShader, programDesc.fragmentShader}) {
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

        return Pipeline{shaderProgram, pipeline};
    }

    void bindPipeline(LLGL::CommandBuffer *cmdBuf, Pipeline &pipeline) {
        cmdBuf->SetPipelineState(*pipeline.state);
    }
}