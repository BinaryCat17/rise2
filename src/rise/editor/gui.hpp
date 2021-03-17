#pragma once

#include "rise/util/ecs.hpp"
#include "rise/components/rendering/imgui.hpp"
#include <map>

namespace rise::editor {
    enum class GuiComponentType {
        DragFloat,
        DragFloat3,
        DragFloat2,
        Text, // std string
    };


    struct TypeTable {
        std::map<flecs::entity_t, GuiComponentType> map;
    };

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context,
            TypeTable &table);

    template<typename T>
    void regGuiComponent(flecs::entity app, GuiComponentType type) {
        auto& table = app.get_mut<TypeTable>()->map;
        table[app.world().type_id<T>()] = type;
    }
}