#include "physic.hpp"
#include "platform.hpp"
#include "rendering.hpp"
#include "system.hpp"

namespace rise {

    using namespace std::chrono_literals;
    const inline std::chrono::milliseconds timeStep = 17ms;

    struct PhysicResources {
        physic::PhysicsCommon common;
        physic::PhysicsWorld *world = nullptr;
        std::chrono::milliseconds accumulator = 0ms;
    };

    physic::Vector3 convert(glm::vec3 v) {
        return physic::Vector3(v.x, v.y, v.z);
    }

    physic::Transform getTransform(entt::registry &r, entt::entity e) {
        auto position = r.get<Position>(e);
        auto rotation = r.get<Rotation>(e);
        physic::Vector3 vector(convert(position.val));

        auto quaternion = physic::Quaternion::fromEulerAngles(
                glm::radians(rotation.val.x),
                glm::radians(rotation.val.y),
                glm::radians(rotation.val.z));

        return {vector, quaternion};
    }

    void updatePhysicBody(entt::registry &r, entt::entity e) {
        auto &resources = r.ctx<PhysicResources>();
        if(auto physicBody = r.try_get<PhysicBody>(e)) {
            auto type = physicBody->type;
            if (auto pBody = r.try_get<physic::RigidBody *>(e)) {
                auto body = *pBody;
                body->setTransform(getTransform(r, e));
            } else {
                auto body = resources.world->createRigidBody(getTransform(r, e));
                body->setType(type);
                r.emplace<physic::RigidBody *>(e, body);
            }
        }
    }

    void updateBoxCollision(entt::registry &r, entt::entity e) {
        auto &resources = r.ctx<PhysicResources>();
        if (auto pBody = r.try_get<physic::RigidBody *>(e)) {
            auto body = *pBody;
            auto collision = r.get<BoxCollision>(e);

            if (auto pBox = r.try_get<physic::BoxShape *>(e)) {
                auto box = *pBox;
                box->setHalfExtents(convert(collision.halfExtent));
            } else {
                auto box = resources.common.createBoxShape(convert(collision.halfExtent));
                auto transform = physic::Transform::identity();
                body->addCollider(box, transform);
                r.emplace<physic::BoxShape *>(e, box);
            }
        }
    }

    void initPhysic(entt::registry &r) {
        auto &resources = r.set<PhysicResources>();
        physic::PhysicsWorld::WorldSettings settings;
        settings.defaultVelocitySolverNbIterations = 20;
        settings.isSleepingEnabled = false;
        settings.gravity = physic::Vector3(0, -9.81, 0);
        resources.world = resources.common.createPhysicsWorld(settings);

        r.on_construct<PhysicBody>().connect<&updatePhysicBody>();
        r.on_update<Position>().connect<&updatePhysicBody>();
        r.on_update<Rotation>().connect<&updatePhysicBody>();
        r.on_construct<BoxCollision>().connect<&updateBoxCollision>();
        r.on_update<BoxCollision>().connect<&updateBoxCollision>();
    }

    void updatePhysic(entt::registry &r) {
        auto &resources = r.ctx<PhysicResources>();

        resources.accumulator += events::pullTime();
        resources.accumulator = std::min(500ms, resources.accumulator);

        while (resources.accumulator >= timeStep) {
            resources.world->update(static_cast<float>(timeStep.count()) / 1000.f);
            resources.accumulator -= timeStep;
        }

        for(auto e : r.view<physic::RigidBody*>())  {
            auto body = r.get<physic::RigidBody*>(e);
            auto const& transform = body->getTransform();
            auto position = transform.getPosition();

            float angle;
            physic::Vector3 rotation;
            transform.getOrientation().getRotationAngleAxis(angle, rotation);
            angle = glm::degrees(angle);

            r.replace<Position>(e, glm::vec3(position.x, position.y, position.z));
            r.replace<Rotation>(e, glm::vec3(rotation.x * angle, rotation.y * angle,
                    rotation.z * angle));
        }
    }
}