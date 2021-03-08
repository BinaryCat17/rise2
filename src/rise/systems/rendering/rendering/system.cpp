#include "system.hpp"
#include "systems/rendering/resources/application.hpp"

namespace rise::systems::rendering {
    void prepareRenderSystem(flecs::iter it) {
        auto &state = it.column<ApplicationResource>(1)->state;

        auto cmd = state.cmdBuf;
        cmd->Begin();
        cmd->BeginRenderPass(*state.context);
        cmd->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRenderSystem(flecs::iter it) {
        auto &state = it.column<ApplicationResource>(1)->state;
        state.cmdBuf->EndRenderPass();
        state.cmdBuf->End();
        state.queue->Submit(*state.cmdBuf);
        state.context->Present();
    }
}
