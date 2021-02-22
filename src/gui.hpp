#pragma once
#include <LLGL/LLGL.h>
#include <entt/entt.hpp>

namespace rise {
    void initGui(entt::registry& r);

    void renderGui(entt::registry& r, LLGL::CommandBuffer* cmdBuf);
}