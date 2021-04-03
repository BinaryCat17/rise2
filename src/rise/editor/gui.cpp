#include "gui.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include "rendering/glm.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "rendering/llgl/math.hpp"
#include <iostream>

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
                    case GuiComponentType::BoolFlag:
                        written = ImGui::Checkbox("", reinterpret_cast<bool *>(pVal));
                        size = sizeof(bool);
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

    void imGuizmoSubmodule(flecs::entity e, rendering::RegTo app, rendering::RenderTo viewport,
            rendering::Position3D position, rendering::Rotation3D rotation,
            rendering::Scale3D scale) {

        ImGuizmo::OPERATION *currentImGuizmoOp = nullptr;

        if (!app.e.get<GuiState>()) {
            return;
        }
        auto state = app.e.mut(e).get_mut<GuiState>();
        if (state->selectedEntity.id() != e.id()) {
            return;
        }

        currentImGuizmoOp = &state->currentOp;

        ImGuizmo::Enable(true);
        ImGuizmo::BeginFrame();

        auto context = app.e.get<rendering::GuiContext>()->context;
        ImGui::SetCurrentContext(context);

        if (ImGui::RadioButton("Translate", *currentImGuizmoOp == ImGuizmo::TRANSLATE))
            *currentImGuizmoOp = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", *currentImGuizmoOp == ImGuizmo::ROTATE))
            *currentImGuizmoOp = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", *currentImGuizmoOp == ImGuizmo::SCALE))
            *currentImGuizmoOp = ImGuizmo::SCALE;
        ImGui::SameLine();
        if (ImGui::Button("Deselect")) {
            state->selectedEntity = flecs::entity(0);
        }

        ImGuiIO &io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

        glm::mat4 modelMatrix;
        ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x,
                &modelMatrix[0][0]);

        auto vPosition = *viewport.e.get<rendering::Position3D>();
        auto vRotation = *viewport.e.get<rendering::Rotation3D>();
        glm::vec3 origin = calcCameraOrigin(toGlm(vPosition), toGlm(vRotation));
        auto view = glm::lookAt(toGlm(vPosition), origin, glm::vec3(0, 1, 0));
        auto projection = glm::perspective(glm::radians(45.0f),
                io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f);

        if (ImGuizmo::Manipulate(&view[0][0], &projection[0][0], *currentImGuizmoOp,
                ImGuizmo::WORLD, &modelMatrix[0][0], nullptr, nullptr)) {
            ImGuizmo::DecomposeMatrixToComponents(&modelMatrix[0][0], &position.x,
                    &rotation.x, &scale.x);
        }

        e.set<rendering::Position3D>(position);
        e.set<rendering::Rotation3D>(rotation);
        e.set<rendering::Scale3D>(scale);
    }

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::editor");
        ecs.component<GuiComponentType>("GuiComponentType");
        ecs.component<GuiComponentDefault>("GuiComponentDefault");
        ecs.component<GuiQuery>("GuiQuery");
        ecs.component<GuiTag>("GuiTag");
        ecs.component<GuiState>("GuiState");
        ecs.set<GuiQuery>({ecs.query<GuiComponentDefault>(), ecs.query<>("GuiTag")});
        ecs.system<>("setGuiState", "rise.rendering.llgl.ApplicationId").kind(flecs::OnSet).
                each([](flecs::entity e) {
            e.set<GuiState>({ImGuizmo::TRANSLATE, flecs::entity(0)});
        });
    }

    void guiSubmodule(flecs::entity e, rendering::RegTo app) {
        auto context = app.e.get<rendering::GuiContext>()->context;
        ImGui::SetCurrentContext(context);

        auto name = e.name();
        if (name.size() && ImGui::TreeNode(name.c_str())) {
            auto ecs = e.world();
            if (app.e.get<GuiState>()) {
                auto state = app.e.mut(e).get_mut<GuiState>();

                if (ImGui::Button("Select")) {
                    state->selectedEntity = e;
                }
            }

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