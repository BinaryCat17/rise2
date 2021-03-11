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

    void initCoreState(flecs::entity e, Path const &path, flecs::Name const &title, Extent2D size) {
        auto renderer = createRenderer();
        auto window = createGameWindow(title.value, toGlm(size));
        e.set(createRenderingContext(renderer.get(), window));
        e.set(createSampler(renderer.get()));
        e.set(renderer->GetCommandQueue());
        e.set(renderer->CreateCommandBuffer());
        e.set(window);
        e.set(std::move(renderer));
    }
}
