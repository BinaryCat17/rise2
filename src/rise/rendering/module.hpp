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

    struct Mesh {};

    struct Texture {};

    struct Material {};

    struct Model {};

    struct PointLight {};

    struct Viewport {};

    struct Module {
        explicit Module(flecs::world &ecs);
    };
}