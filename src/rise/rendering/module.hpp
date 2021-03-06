#pragma once

#include <glm/glm.hpp>
#include "rise/ecs.hpp"

namespace rise {
    struct Viewport {
        glm::vec2 pos;
        glm::vec2 size;
    };

    struct WorldPosition {
        glm::vec3 val{};
    };

    struct WorldRotation {
        glm::vec3 val{};
    };

    struct WorldScale {
        glm::vec3 val{};
    };

    struct DiffuseColor {
        glm::vec3 val{};
    };

    struct Distance {
        float val{};
    };

    struct Intensity {
        float val{};
    };

    struct Path {
        std::string val;
    };

    struct Material {};

    struct PointLight {};

    struct Texture {};

    struct Mesh {};

    struct RenderModule {
        explicit RenderModule(flecs::world &ecs);

        static void loadTexture(flecs::world &ecs, flecs::entity e, std::string const &path);

        static void loadMesh(flecs::world &ecs, flecs::entity e, std::string const &path);

        static void renderTo(flecs::world &ecs, flecs::entity e, flecs::entity viewport);

        static void setWindowSize(flecs::world &ecs, glm::vec2 size);

        static glm::vec2 getWindowSize(flecs::world &ecs);

        static void setWorkDirectory(flecs::world& ecs, std::string const& dir);

        static void setWindowTitle(flecs::world& ecs, std::string const& title);

        static std::string getWindowTitle(flecs::world& ecs);

        static void gui(flecs::world &ecs, std::function<void()> callback);
    };
}
