#include "state.hpp"
#include "systems/rendering/util/debug.hpp"
#include "systems/rendering/util/platform.hpp"
#include "systems/rendering/util/resources.hpp"

namespace rise::systems::rendering {
    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    RenderState createRenderState(std::string const &root, SDL_Window *window) {
        RenderState state;

        state.renderer = createRenderer();
        auto renderer = state.renderer.get();

        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        state.context = createRenderingContext(renderer, window);
        state.window = window;
        state.sampler = createSampler(renderer);
        state.queue = renderer->GetCommandQueue();
        state.cmdBuf = renderer->CreateCommandBuffer();

        return state;
    }
}
