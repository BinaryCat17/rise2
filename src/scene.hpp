#pragma once

#include "mesh.hpp"
#include "resources.hpp"
#include <entt/entt.hpp>

namespace rise {
    unsigned const TransformBinding = 0;

    struct SceneResources {
        std::vector<LLGL::Buffer *> uniformBuffers;
        std::vector<Mesh> meshes;

        struct UniformId {
            size_t index;
            unsigned binding;
        };

        struct MeshId {
            size_t index;
        };
    };

    struct Position : glm::vec3 {};

    struct Rotation : glm::vec3 {};

    struct Scale : glm::vec3 {};

    void makeModel(entt::registry &r, entt::entity entity, size_t meshId, Position position);
}