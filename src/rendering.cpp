#include "rendering.hpp"
#include <iostream>
#include "mesh.hpp"

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

    Pipeline createPipeline(LLGL::RenderSystem *renderer, std::string const &root,
            ShaderResources const &resources) {
        std::string vertPath = root + "/shaders/shader.vert";
        std::string fragPath = root + "/shaders/shader.frag";

        LLGL::ShaderDescriptor VSDesc, FSDesc;
        VSDesc = {LLGL::ShaderType::Vertex, vertPath.data()};
        FSDesc = {LLGL::ShaderType::Fragment, fragPath.data()};

        VSDesc.vertex.inputAttribs = getVertexFormat().attributes;

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
}