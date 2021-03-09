#include "system.hpp"

namespace rise::systems::rendering {
    void prepareRenderSystem(flecs::entity, ApplicationResource& app) {
        auto cmd = app.state.cmdBuf;
        cmd->Begin();
        cmd->BeginRenderPass(*app.state.context);
        cmd->Clear(LLGL::ClearFlags::ColorDepth);
    }

    void submitRenderSystem(flecs::entity, ApplicationResource& app) {
        app.state.cmdBuf->EndRenderPass();
        app.state.cmdBuf->End();
        app.state.queue->Submit(*app.state.cmdBuf);
        app.state.context->Present();
    }
}
