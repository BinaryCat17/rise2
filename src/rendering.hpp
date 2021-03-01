#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace rise {
    struct Parameters {
        std::string root = ".";
        std::string title = "minecraft 2";
        glm::vec2 windowSize = {1600, 1200};
        glm::vec2 windowPos = {400, 0};
    };

    struct RenderSystem {
        static void init(entt::registry &r);

        static bool update(entt::registry &r);

        static void destroy(entt::registry &r);

        static void setActiveCamera(entt::registry &r, entt::entity camera);

        static entt::entity getActiveCamera(entt::registry &r);

        static void loadTexture(entt::registry &r, entt::entity e, std::string const &path);

        static void loadMesh(entt::registry &r, entt::entity e, std::string const &path);

        static entt::delegate<void(entt::registry &r)> &imguiCallback(entt::registry &r);
    };
}