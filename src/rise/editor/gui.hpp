#pragma once

#include <flecs.h>
#include "rise/rendering/imgui.hpp"

namespace rise::editor {
    enum class GuiComponentType {
        Tag,
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
        explicit Module(flecs::world &ecs);
    };

    void guiSubmodule(flecs::entity e, rendering::GuiContext context);

    template<typename T>
    void regGuiComponent(flecs::world &ecs, GuiComponentType type, T init = {}) {
        auto e = ecs.entity(ecs.type_id<T>());
        e.template set<GuiComponentType>(type);
        e.template set<GuiComponentDefault>({new T(init), sizeof(T)});
    }
}