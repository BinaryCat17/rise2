#include "gui.hpp"
#include <misc/cpp/imgui_stdlib.h>

namespace rise::editor {
    struct GuiQuery {
        flecs::query<GuiComponentDefault> component;
        flecs::query<> tag;
    };

    void writeEntityComponents(flecs::world &ecs, flecs::entity e) {
        for (auto t : e.type().vector()) {
            auto type = ecs.entity(t).get<GuiComponentType>();
            if (type && e.owns(t)) {
                auto pVal = ecs_get_mut_w_entity(ecs.c_ptr(), e.id(), t, nullptr);
                auto name = ecs_get_name(ecs.c_ptr(), t);
                bool written = false;
                size_t size = 0;

                ImGui::Text("%s", name);
                ImGui::PushID(name);

                switch (*type) {
                    case GuiComponentType::DragFloat:
                        written = ImGui::DragFloat("", reinterpret_cast<float *>(pVal));
                        size = sizeof(float);
                        break;
                    case GuiComponentType::DragFloat2:
                        written = ImGui::DragFloat2("", reinterpret_cast<float *>(pVal));
                        size = sizeof(float) * 2;
                        break;
                    case GuiComponentType::DragFloat3:
                        written = ImGui::DragFloat3("", reinterpret_cast<float *>(pVal));
                        size = sizeof(float) * 3;
                        break;
                    case GuiComponentType::InputTextStdString:
                        written = ImGui::InputText("", reinterpret_cast<std::string *>(pVal));
                        size = sizeof(std::string);
                        break;
                }
                if (written) {
                    ecs_set_ptr_w_entity(ecs.c_ptr(), e.id(), t, size, pVal);
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    e.remove(t);
                }
                ImGui::PopID();
            }
        }
    }

    void writeEntityTags(flecs::world &ecs, flecs::entity e) {
        for (auto t : e.type().vector()) {
            auto type = ecs.entity(t);
            if (type.has<GuiTag>() && e.owns(t)) {
                auto name = ecs_get_name(ecs.c_ptr(), t);
                ImGui::PushID(name);
                ImGui::Text("%s", name);
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    e.remove(t);
                }
                ImGui::PopID();
            }
        }
    }

    void listComponents(flecs::world &ecs, flecs::entity e) {
        auto const &query = ecs.get<GuiQuery>()->component;
        for (auto it : query) {
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

    void listTags(flecs::world &ecs, flecs::entity e) {
        auto const &query = ecs.get<GuiQuery>()->tag;
        for (auto it : query) {
            for (auto row : it) {
                auto type = it.entity(row);
                if (!e.owns(type)) {
                    if (ImGui::Button(type.name().c_str())) {
                        e.add(type);
                    }
                }
            }
        }
    }

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::editor");
        ecs.component<GuiComponentType>("GuiComponentType");
        ecs.component<GuiComponentDefault>("GuiComponentDefault");
        ecs.component<GuiQuery>("GuiQuery");
        ecs.component<GuiTag>("GuiTag");
        ecs.set<GuiQuery>({ecs.query<GuiComponentDefault>(), ecs.query<>("GuiTag")});
    }

    void guiSubmodule(flecs::entity e, rendering::RegTo state) {
        auto context = state.e.get<rendering::GuiContext>()->context;
        ImGui::SetCurrentContext(context);
        auto ecs = e.world();

        if (!e.name().empty() && ImGui::TreeNode(e.name().c_str())) {
            writeEntityComponents(ecs, e);
            writeEntityTags(ecs, e);

            if (ImGui::Button("Add component")) {
                ImGui::OpenPopup("components_popup");
            }

            if (ImGui::BeginPopup("components_popup")) {
                ImGui::Text("Available components: ");
                listComponents(ecs, e);
                ImGui::EndPopup();
            }

            if (ImGui::Button("Add tag")) {
                ImGui::OpenPopup("tags_popup");
            }

            if (ImGui::BeginPopup("tags_popup")) {
                ImGui::Text("Available tags: ");
                listTags(ecs, e);
                ImGui::EndPopup();
            }

            ImGui::NewLine();
            ImGui::TreePop();
        }
    }
}