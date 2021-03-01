#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>

struct SDL_Window;

namespace rise {
    glm::vec3 calcOrigin(glm::vec3 position, glm::vec3 rotation);

    struct InputSystem {
        static void init(entt::registry &r);

        static bool update(entt::registry &r);

        static void destroy(entt::registry &r);

        static void setRelativeMode(entt::registry &r, bool enable);

        static SDL_Window* getWindow(entt::registry &r);
    };
}