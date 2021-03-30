#pragma once
#include "resources.hpp"

namespace rise::rendering {
    void importViewport(flecs::world &ecs);

    void prepareViewport(flecs::entity, ApplicationRef ref, ViewportId viewportId);

    void updateViewportCamera(flecs::entity, ApplicationRef ref, ViewportId viewportId,
            Extent2D size, Position3D position, Rotation3D rotation);

    void updateViewportLight(flecs::entity, ApplicationRef ref, ViewportRef viewportRef,
            Position3D position, DiffuseColor color, Intensity intensity, Distance distance,
            LightId lightId);

    void finishViewport(flecs::entity, ApplicationRef ref, ViewportId viewportId);
}