#pragma once

#include <glm/glm.hpp>
#include <flecs.h>

namespace rise {
    struct RootDirectory {
        std::string root;
    };

    struct Title {
        std::string title;
    };

    struct WindowSize {
        glm::vec2 size;
    };

    struct WindowPosition {
        glm::vec2 position;
    };

    struct CameraPosition {
        glm::vec3 position;
    };

    struct CameraRotation {
        glm::vec3 rotation;
    };

    struct RenderModule {
        explicit RenderModule(flecs::world &ecs) {
            ecs.module<RenderModule>();
            ecs.set<CameraPosition>({glm::vec3{}});
            ecs.set<CameraRotation>({glm::vec3{}});
        }

        static void loadTexture(flecs::world &ecs, std::string const &path);

        static void loadMesh(flecs::world &ecs, std::string const &path);

        static void addImgui(std::function<void(flecs::world &ecs)> callback);
    };
}