#include "module.hpp"
#include "rendering/module.hpp"
#include "rendering/glm.hpp"
#include "imgui.h"
#include <SDL.h>

namespace rise::input {
    using namespace rendering;

    void move(Position3D &position, glm::vec3 direction, float speed, float time) {
        glm::vec3 pos = toGlm(position);
        pos += speed * direction * time * 1000.f / 17.f;
        position = fromGlmPosition3D(pos);
    }

    glm::vec3 calcOrigin(glm::vec3 position, glm::vec3 rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;
        direction.y = std::sin(glm::radians(rotation.z)) * 3;
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;

        return {position.x + direction.x, position.y + direction.y, position.z + direction.z};
    }

    void processRelative(flecs::entity e) {
        if (ImGui::IsKeyPressed(SDL_SCANCODE_F)) {
            e.set<Relative>({!e.get<Relative>()->enabled});
        }
    }

    void processKeyboard(flecs::entity e, Position3D position, Rotation3D rotation) {
        glm::vec3 origin = calcOrigin(toGlm(position), toGlm(rotation));
        glm::vec3 direction = glm::normalize(toGlm(position) - origin);

        glm::vec3 right = glm::normalize(glm::cross({0.0f, 1.0f, 0.0f}, direction));
        glm::vec3 up = glm::cross(direction, right);
        glm::vec3 front = glm::cross(up, right);

        float const speed = 0.2;


        if (ImGui::IsKeyDown(SDL_SCANCODE_D)) {
            move(position, right, speed, e.delta_time());
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_A)) {
            move(position, -right, speed, e.delta_time());
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_W)) {
            move(position, front, speed, e.delta_time());
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_S)) {
            move(position, -front, speed, e.delta_time());
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_E)) {
            move(position, up, speed, e.delta_time());
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_Q)) {
            move(position, -up, speed, e.delta_time());
        }

        e.set<Position3D>(position);
    }

    void processMouse(flecs::entity e, RegTo app, Rotation3D rotation) {
        float const speed = 0.05f;

        int x, y;
        SDL_GetRelativeMouseState(&x, &y);

        if (app.e.get<Relative>()->enabled) {
            rotation.x += static_cast<float>(x) * speed;
            rotation.z -= static_cast<float>(y) * speed;
        }

        e.set<Rotation3D>(rotation);
    }

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::input");
        ecs.import<rendering::Module>();
        ecs.component<Controllable>("Controllable");

        ecs.system<Position3D, const Rotation3D>("processKeyboard", "Controllable").
                kind(flecs::PostLoad).each(processKeyboard);

        ecs.system<>("processRelative", "OWNED:rise.rendering.Relative").
                kind(flecs::OnLoad).each(processRelative);

        ecs.system<const RegTo, Rotation3D>("processMouse", "Controllable").
                kind(flecs::PostLoad).each(processMouse);
    }
}
