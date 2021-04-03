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

    struct Albedo {
        float r;
        float g;
        float b;
    };

    struct Metallic {
        float val;
    };

    struct Ao {
        float val;
    };

    struct Roughness {
        float val;
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

    struct Title {
        std::string title;
    };

    struct Relative {
        bool enabled;
    };

    struct AlbedoTexture {
        flecs::entity e;
    };

    struct MetallicTexture {
        flecs::entity e;
    };

    struct RoughnessTexture {
        flecs::entity e;
    };

    struct AoTexture {
        flecs::entity e;
    };

    struct RegTo {
        flecs::entity e;
    };

    struct RenderTo {
        flecs::entity e;
    };

    struct Mesh {};

    struct Texture {};

    struct Material {};

    struct Model {};

    struct PointLight {};

    struct Viewport {};

    struct Shadow {};

    struct Module {
        explicit Module(flecs::world &ecs);
    };
}