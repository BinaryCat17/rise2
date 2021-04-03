#pragma once

#include <flecs.h>
#include "rise/rendering/imgui.hpp"
#include "rise/rendering/module.hpp"
#include <ImGuizmo.h>

namespace rise::editor {
    enum class GuiComponentType {
        BoolFlag,
        DragFloat,
        DragFloat3,
        DragFloat2,
        InputTextStdString,
    };

    struct GuiComponentDefault {
        void const *val;
        size_t size;
    };

    struct GuiTag {};

    struct GuiState {
        ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
        flecs::entity selectedEntity;
    };

    struct Module {
        explicit Module(flecs::world &ecs);
    };

    void guiSubmodule(flecs::entity e, rendering::RegTo state);

    void imGuizmoSubmodule(flecs::entity e, rendering::RegTo app, rendering::RenderTo viewport,
            rendering::Position3D position, rendering::Rotation3D rotation,
            rendering::Scale3D scale);

    template<typename T>
    void regGuiComponent(flecs::world &ecs, GuiComponentType type, T init = {}) {
        auto e = ecs.entity(ecs.type_id<T>());
        e.template set<GuiComponentType>(type);
        e.template set<GuiComponentDefault>({new T(init), sizeof(T)});
    }

    template<typename T>
    void regGuiTag(flecs::world &ecs) {
        auto e = ecs.entity(ecs.type_id<T>());
        e.template add<GuiTag>();
    }
}