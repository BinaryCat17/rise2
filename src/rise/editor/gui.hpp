#pragma once
#include "rise/util/ecs.hpp"
#include "rise/components/rendering/imgui.hpp"

namespace rise::editor {
    enum class GuiComponentType {
        DragFloat,
        DragFloat3,
        DragFloat2,
        Text, // std string
    };

    using TypeTable = flecs::map<flecs::entity_t, GuiComponentType>;

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context,
            TypeTable const& table);
}