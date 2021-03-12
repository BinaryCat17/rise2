#include "systems.hpp"
#include "platform.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include "components/rendering/glm.hpp"

namespace rise::systems::rendering {
    void prepareRender(flecs::entity, CommandBuffer cmdBuf, Context context) {
        cmdBuf.val->Begin();
        cmdBuf.val->BeginRenderPass(*context.val);
        cmdBuf.val->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRender(flecs::entity, CommandBuffer cmdBuf, Queue queue, Context context) {
        cmdBuf.val->EndRenderPass();
        cmdBuf.val->End();
        queue.val->Submit(*cmdBuf.val);
        context.val->Present();
    }

    void updateWindowSize(flecs::entity, Window window, Context context, Extent2D size) {
        SDL_SetWindowSize(window.window, static_cast<int>(size.width),
                static_cast<int>(size.height));
        context.val->SetVideoMode({{static_cast<uint32_t>(size.width),
                static_cast<uint32_t>(size.height)}});
    }

    std::shared_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void initCoreState(flecs::entity e) {
        auto renderer = createRenderer();
        auto window = createGameWindow(
                checkGet<flecs::Name>(e).value,
                toGlm(checkGet<Extent2D>(e)));
        e.set(Context{createRenderingContext(renderer.get(), window)});
        e.set(Sampler{createSampler(renderer.get())});
        e.set(Queue{renderer->GetCommandQueue()});
        e.set(CommandBuffer{renderer->CreateCommandBuffer()});
        e.set(Window{window});
        e.set<RenderSystem>(renderer);
    }
}
