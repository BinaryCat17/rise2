#include "core.hpp"
#include "utils.hpp"

namespace rise::rendering {
    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            //      std::cout << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            //     std::cout << message.GetText() << std::endl;
        }
    };

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallback(
                [](LLGL::Log::ReportType type, const std::string &message,
                        const std::string &contextInfo, void *userData) {
                    switch (type) {
                        case LLGL::Log::ReportType::Error:
                            std::cerr << "Error: ";
                            std::cerr << message << std::endl;
                            break;
                        case LLGL::Log::ReportType::Warning:
                            std::cerr << "Warning: ";
                            std::cerr << message << std::endl;
                            break;
                        case LLGL::Log::ReportType::Information:
                            break;
                        case LLGL::Log::ReportType::Performance:
                            break;
                    }
                });
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }


    void initCoreRenderer(flecs::entity, ApplicationState &state) {
        auto &core = state.core;
        core.renderer = createRenderer();
    }

    void initCoreState(flecs::entity e, ApplicationState &state) {
        auto &core = state.core;
        core.sampler = createSampler(core.renderer.get());
        core.queue = core.renderer->GetCommandQueue();
        core.cmdBuf = core.renderer->CreateCommandBuffer();

        auto renderer = state.core.renderer.get();

        e.set<RegTo>({e});
    }

    void importCoreState(flecs::world &ecs) {
    }

    void prepareRender(flecs::entity, ApplicationId app) {
        auto &core = app.id->core;

        core.cmdBuf->Begin();
    }


    void prepareColorPass(flecs::entity, ApplicationId app) {
        auto &core = app.id->core;
        core.cmdBuf->BeginRenderPass(*app.id->platform.context);
        core.cmdBuf->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void endColorPass(flecs::entity, ApplicationId app) {
        auto &core = app.id->core;
        core.cmdBuf->EndRenderPass();
    }

    void submitRender(flecs::entity, ApplicationId app) {
        auto &core = app.id->core;
        core.cmdBuf->End();
        core.queue->Submit(*core.cmdBuf);
        app.id->platform.context->Present();
    }
}