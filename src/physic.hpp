#pragma once
#include <LLGL/LLGL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <reactphysics3d/reactphysics3d.h>

namespace rise {
    namespace physic = reactphysics3d;

    struct PhysicBody {
        physic::BodyType type = physic::BodyType::STATIC;
    };

    struct BoxCollision {
        glm::vec3 halfExtent = {};
    };

    void initPhysic(entt::registry& r);

    void updatePhysic(entt::registry& r);
}