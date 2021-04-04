#pragma once
#include "rise/rendering/glm.hpp"
#include "rise/rendering/module.hpp"
#include <reactphysics3d/reactphysics3d.h>

namespace rise::physics {
    namespace rp = reactphysics3d;

    struct PhysicsState {
        rp::PhysicsCommon common;
        rp::PhysicsWorld *world = nullptr;
        uint32_t accumulator = 0; // ms
    };

    struct PhysicsId {
        PhysicsState *id;
    };

    using BodyType = rp::BodyType;

    struct PhysicBody {
        rp::BodyType type = rp::BodyType::STATIC;
    };

    struct BoxCollision {
        glm::vec3 halfExtent = {};
    };

    struct Velocity {
        float x;
        float y;
        float z;
    };

    struct Mass {
        float kg;
    };

    struct SphereCollision {
        float radius;
    };

    struct Module {
        explicit Module(flecs::world &ecs);
    };
}