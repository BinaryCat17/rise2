#include "gui.hpp"

namespace rise::editor {
    void regDragFloat(flecs::world &ecs, flecs::entity e, flecs::entity_t t) {
        auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
        ImGui::DragFloat(ecs_get_name(ecs.c_ptr(), t), reinterpret_cast<float *>(pVal));
        ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, sizeof(float), pVal);
    }

    void regDragFloat2(flecs::world &ecs, flecs::entity e, flecs::entity_t t) {
        auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
        ImGui::DragFloat2(ecs_get_name(ecs.c_ptr(), t), reinterpret_cast<float *>(pVal));
        ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, sizeof(float) * 2, pVal);
    }

    void regDragFloat3(flecs::world &ecs, flecs::entity e, flecs::entity_t t) {
        auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
        ImGui::DragFloat3(ecs_get_name(ecs.c_ptr(), t), reinterpret_cast<float *>(pVal));
        ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, sizeof(float) * 3, pVal);
    }

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context,
            TypeTable &table) {
        auto ecs = e.world();

        for (auto t : e.type().vector()) {
            switch (table.get(t)) {
                case GuiComponentType::DragFloat:
                    regDragFloat(ecs, e, t);
                    break;
                case GuiComponentType::DragFloat3:
                    regDragFloat3(ecs, e, t);
                    break;
                case GuiComponentType::DragFloat2:
                    regDragFloat2(ecs, e, t);
                    break;
                case GuiComponentType::Text:
                    break;
            }

            e.get(t);
        }
    }
}