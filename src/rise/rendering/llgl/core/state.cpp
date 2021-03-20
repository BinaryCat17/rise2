#include "state.hpp"
#include "platform.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include "../module.hpp"
#include "rise/rendering/glm.hpp"

namespace rise::rendering {
    void updateWindowSize(flecs::entity, CoreState &core, rendering::Extent2D size) {
        SDL_SetWindowSize(core.window, static_cast<int>(size.width),
                static_cast<int>(size.height));
        core.context->SetVideoMode({static_cast<uint32_t>(size.width),
                static_cast<uint32_t>(size.height)});
    }

    void regCoreState(flecs::entity e) {
        if(e.owns<LLGLApplication>()) {
            if (!e.has<Path>()) {
                e.set<Path>({"./rendering"});
            }
            e.set<CoreState>({});
        }
    }

    std::shared_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void initCoreState(flecs::entity e, CoreState &core) {
        core.renderer = createRenderer();
        core.root = e.get<Path>()->file;
        core.window = createGameWindow(e.name(), toGlm(*e.get<Extent2D>()));
        core.context = createRenderingContext(core.renderer.get(), core.window);
        core.sampler = createSampler(core.renderer.get());
        core.queue = core.renderer->GetCommandQueue();
        core.cmdBuf = core.renderer->CreateCommandBuffer();
    }

    void updateRelative(flecs::entity, CoreState &core, Relative val) {
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(val.enabled));
    }

    void importCoreState(flecs::world &ecs) {
        ecs.system<>("regCoreState", "rise.rendering.llgl.Application").
                kind(flecs::OnAdd).each(regCoreState);

        ecs.system<CoreState>("initCoreState", "OWNED:CoreState").
                kind(flecs::OnSet).each(initCoreState);

        ecs.system<CoreState, const Extent2D>("updateCoreStateWindowSize", "OWNED:CoreState").
                kind(flecs::OnSet).each(updateWindowSize);

        ecs.system<CoreState, const Relative>("updateCoreStateRelative",
                "OWNED:CoreState").kind(flecs::OnSet).each(updateRelative);
    }
}