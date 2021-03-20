#include "systems.hpp"

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
}