#include "systems.hpp"
#include "platform.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include "rise/rendering/glm.hpp"

namespace rise::rendering {
    void prepareRender(flecs::entity, CoreState &core) {
        core.cmdBuf->Begin();
        core.cmdBuf->BeginRenderPass(*core.context);
        core.cmdBuf->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRender(flecs::entity, CoreState &core) {
        core.cmdBuf->EndRenderPass();
        core.cmdBuf->End();
        core.queue->Submit(*core.cmdBuf);
        core.context->Present();
    }

    void updateWindowSize(flecs::entity, CoreState &core, Extent2D size) {
        SDL_SetWindowSize(core.window, static_cast<int>(size.width),
                static_cast<int>(size.height));
        core.context->SetVideoMode({{static_cast<uint32_t>(size.width),
                static_cast<uint32_t>(size.height)}});
    }

    std::shared_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void initCoreState(flecs::entity e) {
        CoreState core;
        if (e.has<Path>()) {
            core.path = e.get<Path>()->file;
        } else {
            core.path = "./rendering";
        }

        core.renderer = createRenderer();
        core.window = createGameWindow(checkGet<flecs::Name>(e).value,
                toGlm(checkGet<Extent2D>(e)));
        core.context = createRenderingContext(core.renderer.get(), core.window);
        core.sampler = createSampler(core.renderer.get());
        core.queue = core.renderer->GetCommandQueue();
        core.cmdBuf = core.renderer->CreateCommandBuffer();
        e.set<CoreState>(core);
    }
}
