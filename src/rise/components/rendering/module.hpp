#pragma once

#include "rise/util/ecs.hpp"

ECS_STRUCT(RPosition2D, {
    float x;
    float y;
})

ECS_STRUCT(RRotation2D, {
    float x;
    float y;
})

ECS_STRUCT(RScale2D, {
    float x;
    float y;
})

ECS_STRUCT(RExtent2D, {
    float width;
    float height;
})

ECS_STRUCT(RPosition3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RRotation3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RScale3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RExtent3D, {
    float width;
    float height;
    float depth;
})

ECS_STRUCT(RDiffuseColor, {
    float r;
    float g;
    float b;
})

ECS_STRUCT(RDistance, {
    float meters;
})

ECS_STRUCT(RIntensity, {
    float factor;
})

ECS_STRUCT(RPath, {
    std::string file;
})

ECS_STRUCT(RRelative, {
    bool enabled;
})

ECS_STRUCT(RDiffuseTextureRes, {
    flecs::entity e;
})

namespace rise::components {
    namespace rendering {
        // components
        using Extent2D = RExtent2D;
        using Position2D = RPosition2D;
        using Rotation2D = RRotation2D;
        using Scale2D = RScale2D;
        using Extent3D = RExtent3D;
        using Position3D = RPosition3D;
        using Rotation3D = RRotation3D;
        using Scale3D = RScale3D;
        using DiffuseColor = RDiffuseColor;
        using Distance = RDistance;
        using Intensity = RIntensity;
        using Path = RPath;
        using Relative = RRelative;
        using DiffuseTexture = RDiffuseTextureRes;
    }

    struct Rendering {
        explicit Rendering(flecs::world &ecs);
    };
}
