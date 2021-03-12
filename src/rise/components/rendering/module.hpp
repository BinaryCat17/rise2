#pragma once

#include "rise/util/ecs.hpp"

ECS_STRUCT(RiseRenderingPosition2D, {
    float x;
    float y;
})

ECS_STRUCT(RiseRenderingRotation2D, {
    float x;
    float y;
})

ECS_STRUCT(RiseRenderingScale2D, {
    float x;
    float y;
})

ECS_STRUCT(RiseRenderingExtent2D, {
    float width;
    float height;
})

ECS_STRUCT(RiseRenderingPosition3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RiseRenderingRotation3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RiseRenderingScale3D, {
    float x;
    float y;
    float z;
})

ECS_STRUCT(RiseRenderingExtent3D, {
    float width;
    float height;
    float depth;
})

ECS_STRUCT(RiseRenderingDiffuseColor, {
    float r;
    float g;
    float b;
})

ECS_STRUCT(RiseRenderingDistance, {
    float meters;
})

ECS_STRUCT(RiseRenderingIntensity, {
    float factor;
})

ECS_STRUCT(RiseRenderingPath, {
    flecs::string file;
})

namespace rise::components {
    namespace rendering {
        // components
        using Extent2D = RiseRenderingExtent2D;
        using Position2D = RiseRenderingPosition2D;
        using Rotation2D = RiseRenderingRotation2D;
        using Scale2D = RiseRenderingScale2D;
        using Extent3D = RiseRenderingExtent3D;
        using Position3D = RiseRenderingPosition3D;
        using Rotation3D = RiseRenderingRotation3D;
        using Scale3D = RiseRenderingScale3D;
        using DiffuseColor = RiseRenderingDiffuseColor;
        using Distance = RiseRenderingDistance;
        using Intensity = RiseRenderingIntensity;
        using Path = RiseRenderingPath;
    }

    struct Rendering {
        explicit Rendering(flecs::world &ecs);
    };
}
