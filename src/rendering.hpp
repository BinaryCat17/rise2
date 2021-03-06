#pragma once

#include <glm/glm.hpp>
#include <flecs.h>

namespace rise {
    struct RootDirectory {
        std::string val;
    };

    struct Title {
        std::string val;
    };

    struct WindowSize {
        glm::vec2 val;
    };

    struct WindowPosition {
        glm::vec2 val;
    };

    struct CameraPosition {
        glm::vec3 val;
    };

    struct CameraRotation {
        glm::vec3 val;
    };

    struct WorldPosition {
        glm::vec3 val;
    };

    struct Position {
        glm::vec3 val;
    };

    struct DiffuseColor {
        glm::vec3 val;
    };

    struct Distance {
        float val;
    };

    struct Intensity {
        float val;
    };

    struct PointLight {};

    struct RenderModule {
        explicit RenderModule(flecs::world &ecs);

        static void loadTexture(flecs::world &ecs, flecs::entity e, std::string const &path);

        static void loadMesh(flecs::world &ecs, flecs::entity e, std::string const &path);

        static void addImgui(std::function<void(flecs::world &ecs)> callback);
    };
}