#include "core.hpp"
#include "utils.hpp"

namespace rise::rendering {
    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            std::cout << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            std::cout << message.GetText() << std::endl;
        }
    };

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void initCoreRenderer(flecs::entity e, ApplicationState& state) {
        auto& core = state.core;
        core.renderer = createRenderer();
    }

    void initCoreState(flecs::entity e, ApplicationState& state) {
        auto& core = state.core;
        core.sampler = createSampler(core.renderer.get());
        core.queue = core.renderer->GetCommandQueue();
        core.cmdBuf = core.renderer->CreateCommandBuffer();
        e.set<RegTo>({e});
    }

    void importCoreState(flecs::world &ecs) {
    }

    void prepareRender(flecs::entity, ApplicationId app) {
        auto& core = app.id->core;

        core.cmdBuf->Begin();
        core.cmdBuf->BeginRenderPass(*app.id->platform.context);
        core.cmdBuf->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRender(flecs::entity, ApplicationId app) {
        auto& core = app.id->core;

        core.cmdBuf->Begin();
        core.cmdBuf->BeginRenderPass(*app.id->platform.context);
        core.cmdBuf->Clear(LLGL::ClearFlags::ColorDepth);
    }
}