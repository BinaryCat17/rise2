#include "systems.hpp"
#include "platform.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include "components/rendering/glm.hpp"

namespace rise::systems::rendering {
    void prepareRender(flecs::entity, CommandBuffer cmdBuf, Context context) {
        cmdBuf->Begin();
        cmdBuf->BeginRenderPass(*context);
        cmdBuf->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRender(flecs::entity, CommandBuffer cmdBuf, Queue queue, Context context) {
        cmdBuf->EndRenderPass();
        cmdBuf->End();
        queue->Submit(*cmdBuf);
        context->Present();
    }

    void updateWindowSize(flecs::entity, Window window, Context context, Extent2D size) {
        SDL_SetWindowSize(window, static_cast<int>(size.width), static_cast<int>(size.height));
        context->SetVideoMode({{static_cast<uint32_t>(size.width),
                static_cast<uint32_t>(size.height)}});
    }

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void initCoreState(flecs::entity, RenderSystem& renderer, Window& window, Context& context,
            Sampler& sampler, Queue& queue, CommandBuffer& cmdBuf, Path const &path,
            flecs::Name const &title, Extent2D size) {
        renderer = createRenderer();
        window = createGameWindow(title.value, toGlm(size));
        context = createRenderingContext(renderer.get(), window);
        sampler = createSampler(renderer.get());
        queue = renderer->GetCommandQueue();
        cmdBuf = renderer->CreateCommandBuffer();
    }
}
