#pragma once

#include <flecs.h>
#include "rise/rendering/imgui.hpp"
#include "rise/rendering/module.hpp"
#include <ImGuizmo.h>
#include "rise/physics/module.hpp"

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

    struct GuiState : public rp3d::RaycastCallback {
        GuiState(ImGuizmo::OPERATION currentOp, const flecs::entity &selectedEntity) : currentOp(
                currentOp), selectedEntity(selectedEntity) {}

        rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo &info) override {
            auto e = reinterpret_cast<flecs::entity*>(info.body->getUserData());
            if(*e == selectedEntity) {
                return rp3d::decimal(1.0);
            } else {
                selectedEntity = *e;
                return rp3d::decimal(0.0);
            }
        }

        ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
        flecs::entity selectedEntity;
    };

    struct GuiId {
        GuiState* id;
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