#pragma once

#include "rise/util/ecs.hpp"
#include "rise/components/rendering/imgui.hpp"

namespace rise::editor {
    enum class GuiComponentType {
        DragFloat,
        DragFloat3,
        DragFloat2,
        InputTextStdString,
    };

    struct GuiComponentDefault {
        void const *val;
        size_t size;
    };

    struct Module {
        explicit Module(flecs::world& ecs);
    };

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context);

    template<typename T>
    void regGuiComponent(flecs::world &ecs, GuiComponentType type, T init = {}) {
        auto e = ecs.entity(ecs.type_id<T>());
        e.template set<GuiComponentType>(type);
        e.template set<GuiComponentDefault>({new T(init), sizeof(T)});
    }
}