#pragma once
#include <imgui.h>
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct GuiParameters {
        LLGL::Buffer* val;
    };

    using GuiContext = ImGuiContext*;
}
