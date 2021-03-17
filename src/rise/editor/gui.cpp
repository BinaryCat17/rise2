#include "gui.hpp"
#include <misc/cpp/imgui_stdlib.h>

namespace rise::editor {
    void writeEntities(flecs::world &ecs, flecs::entity e, TypeTable &table) {
        for (auto t : e.type().vector()) {
            if (auto type = table.map.find(t); e.owns(t) && type != table.map.end()) {
                auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
                auto name = ecs_get_name(ecs.c_ptr(), t);
                bool written = false;
                size_t size = 0;

                switch (type->second) {
                    case GuiComponentType::DragFloat:
                        written = ImGui::DragFloat(name, reinterpret_cast<float *>(pVal));
                        size = sizeof(float);
                        break;
                    case GuiComponentType::DragFloat2:
                        written = ImGui::DragFloat2(name, reinterpret_cast<float *>(pVal));
                        size = sizeof(float) * 2;
                        break;
                    case GuiComponentType::DragFloat3:
                        written = ImGui::DragFloat3(name, reinterpret_cast<float *>(pVal));
                        size = sizeof(float) * 3;
                        break;
                    case GuiComponentType::Text:
                        written = ImGui::InputText(name, reinterpret_cast<std::string *>(pVal));
                        size = sizeof(std::string);
                        break;
                }
                if(written) {
                    ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, size, pVal);
                }
            }
        }
    }

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context,
            TypeTable &table) {
        ImGui::SetCurrentContext(context.context);
        auto ecs = e.world();

        if (!e.name().empty() && ImGui::TreeNode(e.name().c_str())) {
            writeEntities(ecs, e, table);
            ImGui::NewLine();
            ImGui::TreePop();
        }
    }
}