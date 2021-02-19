#include "resources.hpp"
#include "shader.hpp"

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

    LLGL::RenderContext *createContext(LLGL::RenderSystem *renderer,
            unsigned width, unsigned height) {
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

        return context;

    }

    Instance createInstance(unsigned width, unsigned height) {
        Instance instance = {};
        instance.renderer = createRenderer();
        instance.context = createContext(instance.renderer.get(), width, height);

        auto &window = LLGL::CastTo<LLGL::Window>(instance.context->GetSurface());
        window.SetTitle(L"Rise!");
        window.Show();
        instance.window = &window;

        return instance;
    }

    void initRenderer(entt::registry &r, unsigned width, unsigned height, std::string const &root) {
        auto const &instance = r.set<Instance>(createInstance(width, height));

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.shaderProgram = shader::program(instance.renderer.get(), root);
        pipelineDesc.pipelineLayout = shader::layout(instance.renderer.get());
        pipelineDesc.rasterizer.multiSampleEnabled = true;

        auto pipeline = instance.renderer->CreatePipelineState(pipelineDesc);


        return Pipeline{shaderProgram, pipeline};
    }
}
