#include "gui.hpp"
#include <misc/cpp/imgui_stdlib.h>

namespace rise::editor {
    struct GuiComponentsQuery {
        flecs::query<GuiComponentDefault> query;
    };

    void writeEntity(flecs::world &ecs, flecs::entity e) {
        for (auto t : e.type().vector()) {
            auto type = ecs.entity(t).get<GuiComponentType>();
            if (type && e.owns(t)) {
                auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
                auto name = ecs_get_name(ecs.c_ptr(), t);
                bool written = false;
                size_t size = 0;

                switch (*type) {
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
                    case GuiComponentType::InputTextStdString:
                        written = ImGui::InputText(name, reinterpret_cast<std::string *>(pVal));
                        size = sizeof(std::string);
                        break;
                }
                if (written) {
                    ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, size, pVal);
                }
            }
        }
    }

    void writeComponents(flecs::world &ecs, flecs::entity e) {
        auto query = ecs.get<GuiComponentsQuery>();
        for (auto it : query->query) {
            auto defaults = it.column<const GuiComponentDefault>(1);
            for (auto row : it) {
                auto type = it.entity(row);
                if (!e.owns(type)) {
                    if (ImGui::Button(type.name().c_str())) {
                        auto init = defaults[row];
                        ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), type.id(), init.size, init.val);
                    }
                }
            }
        }
    }

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("Editor");
        ecs.component<GuiComponentType>("GuiComponentType");
        ecs.component<GuiComponentDefault>("GuiComponentDefault");
        ecs.component<GuiComponentsQuery>("GuiComponentsQuery");
        ecs.set<GuiComponentsQuery>({ecs.query<GuiComponentDefault>()});
    }

    void editorGuiSubmodule(flecs::entity e, components::rendering::GuiContext context) {
        ImGui::SetCurrentContext(context.context);
        auto ecs = e.world();

        if (!e.name().empty() && ImGui::TreeNode(e.name().c_str())) {
            writeEntity(ecs, e);

            if (ImGui::Button("Add component")) {
                ImGui::OpenPopup("components_popup");
            }

            if (ImGui::BeginPopup("components_popup")) {
                ImGui::Text("Available components: ");
                writeComponents(ecs, e);
                ImGui::EndPopup();
            }

            ImGui::NewLine();
            ImGui::TreePop();
        }
    }

}