#include <glm/gtx/euler_angles.hpp>
#include "module.hpp"

namespace rise::physics {
    using namespace std::chrono_literals;
    const uint32_t timeStep = 17; // ms

    struct PhysicBodyId {
        rp::RigidBody *id;
    };

    struct BoxCollisionId {
        rp::BoxShape *id;
    };

    struct SphereCollisionId {
        rp::SphereShape *id;
    };

    rp::Vector3 convert(glm::vec3 v) {
        return rp::Vector3(v.x, v.y, v.z);
    }

    rp::Transform getTransform(rendering::Position3D pos, rendering::Rotation3D rot) {
        rp::Vector3 vector(convert(toGlm(pos)));

        float angle = std::max({rot.x, rot.y, rot.z});
        rp::Quaternion qt;
        if (angle != 0) {
            auto q = glm::angleAxis(glm::radians(angle), glm::normalize(toGlm(rot)));
            qt.x = q.x;
            qt.y = q.y;
            qt.z = q.z;
            qt.w = q.w;
        } else {
            qt = rp::Quaternion::fromEulerAngles({0, 0, 0});
        }

        return {vector, qt};
    }

    void updateRigidBody(flecs::entity e, rendering::RegTo app, PhysicBody body) {
        auto state = app.e.get<PhysicsId>();
        if (auto id = e.get<PhysicBodyId>()) {
            id->id->setType(body.type);
        } else {
            rendering::Position3D pos{0, 0, 0};
            if (auto pPos = e.get<rendering::Position3D>()) pos = *pPos;

            rendering::Rotation3D rot{0, 0, 0};
            if (auto pRot = e.get<rendering::Rotation3D>()) rot = *pRot;

            auto rpBody = state->id->world->createRigidBody(getTransform(pos, rot));
            rpBody->setLinearDamping(0.1);
            rpBody->setAngularDamping(0.1);
            rpBody->setType(body.type);
            rpBody->setUserData(new flecs::entity(e));
            e.set<PhysicBodyId>({rpBody});
        }
    }

    void updateRigidBodyTransform(flecs::entity, PhysicBodyId body, rendering::Position3D pos,
            rendering::Rotation3D rot) {
        body.id->setTransform(getTransform(pos, rot));
    }

    void updateVelocity(flecs::entity, PhysicBodyId body, Velocity velocity) {
        body.id->setLinearVelocity({velocity.x, velocity.y, velocity.z});
    }

    void updateMass(flecs::entity, PhysicBodyId body, Mass mass) {
        body.id->setMass(mass.kg);
    }

    void updateBoxCollision(flecs::entity e, rendering::RegTo app, PhysicBodyId body,
            BoxCollision collision) {
        auto state = app.e.get<PhysicsId>();
        if (auto id = e.get<BoxCollisionId>()) {
            id->id->setHalfExtents(convert(collision.halfExtent));
        } else {
            auto box = state->id->common.createBoxShape(convert(collision.halfExtent));
            auto transform = rp::Transform::identity();
            body.id->addCollider(box, transform)->getMaterial().setFrictionCoefficient(0.8);
            e.set<BoxCollisionId>({box});
        }
    }

    void updateSphereCollision(flecs::entity e, rendering::RegTo app, PhysicBodyId body,
            SphereCollision collision) {
        auto state = app.e.get<PhysicsId>();
        if (auto id = e.get<SphereCollisionId>()) {
            id->id->setRadius(collision.radius);
        } else {
            auto box = state->id->common.createSphereShape(collision.radius);
            auto transform = rp::Transform::identity();
            body.id->addCollider(box, transform)->getMaterial().setFrictionCoefficient(0.8);
            e.set<SphereCollisionId>({box});
        }
    }

    void updatePhysicsTime(flecs::entity e, PhysicsId id) {
        auto &state = *id.id;
        state.accumulator += static_cast<uint32_t>(e.delta_time() * 1000);
        state.accumulator = std::min(500u, state.accumulator);

        while (state.accumulator >= timeStep) {
            state.world->update(static_cast<float>(timeStep) / 1000.f);
            state.accumulator -= timeStep;
        }
    }

    void updatePhysic(flecs::entity e, PhysicBodyId body) {
        auto const &transform = body.id->getTransform();
        auto position = transform.getPosition();

        float angle;
        rp::Vector3 rotation;
        transform.getOrientation().getRotationAngleAxis(angle, rotation);

        e.set<rendering::Position3D>({position.x, position.y, position.z});
        e.set<rendering::Rotation3D>({
                glm::degrees(rotation.x * angle),
                glm::degrees(rotation.y * angle),
                glm::degrees(rotation.z * angle)});
    }

    void initPhysicState(flecs::entity e) {
        rp::PhysicsWorld::WorldSettings settings;
        settings.defaultVelocitySolverNbIterations = 20;
        settings.isSleepingEnabled = false;
        settings.gravity = rp::Vector3(0, -9.81, 0);

        auto state = new PhysicsState{};
        state->world = state->common.createPhysicsWorld(settings);
        e.set<PhysicsId>({state});
    }

    Module::Module(flecs::world &ecs) {
        ecs.module<Module>("rise::physics");
        ecs.import<rise::rendering::Module>();
        ecs.component<PhysicsId>();
        ecs.component<PhysicBody>("PhysicBody");
        ecs.component<PhysicBodyId>("PhysicBodyId");
        ecs.component<BoxCollision>("BoxCollision");
        ecs.component<BoxCollisionId>("BoxCollisionId");
        ecs.component<SphereCollision>("SphereCollision");
        ecs.component<SphereCollisionId>("SphereCollisionId");

        ecs.system<>("initPhysicState", "rise.rendering.llgl.Application").kind(flecs::OnAdd).
                each(initPhysicState);

        ecs.system<const rendering::RegTo, const PhysicBody>("updateRigidBody").
                kind(flecs::OnSet).each(updateRigidBody);

        ecs.system<const PhysicBodyId, const rendering::Position3D, const rendering::Rotation3D>(
                "updateRigidBodyTransform").kind(flecs::OnSet).each(updateRigidBodyTransform);

        ecs.system<const rendering::RegTo, const PhysicBodyId, const BoxCollision>(
                "updateBoxCollision").kind(flecs::OnSet).each(updateBoxCollision);

        ecs.system<const rendering::RegTo, const PhysicBodyId, const SphereCollision>(
                "updateSphereCollision").kind(flecs::OnSet).each(updateSphereCollision);

        ecs.system<const PhysicBodyId, const Velocity>(
                "updateVelocity").kind(flecs::OnSet).each(updateVelocity);

        ecs.system<const PhysicBodyId, const Mass>(
                "updateMass").kind(flecs::OnSet).each(updateMass);

        ecs.system<const PhysicsId>().each(updatePhysicsTime).
                each(updatePhysicsTime);

        ecs.system<PhysicBodyId>("updatePhysics", "[in] OWNED:PhysicBody").each(updatePhysic);
    }
}