#pragma once

#include <flecs.h>
#include <string>
#include <memory>

namespace rise::rendering {
    struct Position2D {
        float x;
        float y;
    };

    struct Rotation2D {
        float x;
        float y;
    };

    struct Scale2D {
        float x;
        float y;
    };

    struct Extent2D {
        float width;
        float height;
    };

    struct Position3D {
        float x;
        float y;
        float z;
    };

    struct Rotation3D {
        float x;
        float y;
        float z;
    };

    struct Scale3D {
        float x;
        float y;
        float z;
    };

    struct Extent3D {
        float width;
        float height;
        float depth;
    };

    struct DiffuseColor {
        float r;
        float g;
        float b;
    };

    struct Distance {
        float meters;
    };

    struct Intensity {
        float factor;
    };

    struct Path {
        std::string file;
    };

    struct Relative {
        bool enabled;
    };

    struct DiffuseTexture {
        flecs::entity e;
    };

    struct Module {
        explicit Module(flecs::world &ecs);
    };

    struct ApplicationT {
        virtual ~ApplicationT() = default;

        virtual void regMesh(flecs::entity app, flecs::entity e) = 0;

        virtual void regTexture(flecs::entity app, flecs::entity e) = 0;

        virtual void regMaterial(flecs::entity app, flecs::entity e) = 0;

        virtual void regModel(flecs::entity app, flecs::entity e) = 0;

        virtual void regPointLight(flecs::entity app, flecs::entity e) = 0;

        virtual void regViewport(flecs::entity app, flecs::entity e) = 0;
    };

    using Application = std::shared_ptr<ApplicationT>;

    void regMesh(flecs::entity app, flecs::entity e);

    void regTexture(flecs::entity app, flecs::entity e);

    void regMaterial(flecs::entity app, flecs::entity e);

    void regModel(flecs::entity app, flecs::entity e);

    void regPointLight(flecs::entity app, flecs::entity e);

    void regViewport(flecs::entity app, flecs::entity e);
}