#pragma once

#include "../core/resources.hpp"
#include "components/rendering/module.hpp"
#include "mesh.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateResourceHeap(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            DiffuseTextureRes diffuse, MaterialRes material, ViewportRes viewport);

    void updateTransform(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            Position3D position, Rotation3D rotation, Scale3D scale);

    void updateMaterial(flecs::entity, CoreState &core, SceneState &scene, MaterialRes material,
            DiffuseColor color);

    void dirtyViewportCamera(flecs::entity, ViewportRes& viewport);

    void dirtyViewportLight(flecs::entity, ViewportRes& viewport);

    void prepareViewport(flecs::entity, CoreState &core, ViewportRes &viewport);

    void updateViewportCamera(flecs::entity, ViewportRes &viewport, Extent2D size,
            Position3D position, Rotation3D rotation);

    void updateViewportLight(flecs::entity, ViewportRes &viewport, Position3D position,
            DiffuseColor color, Intensity intensity, Distance distance);

    void finishViewport(flecs::entity, CoreState &core, ViewportRes &viewport);

    void renderScene(flecs::entity, CoreState &core, SceneState &scene, Position2D position,
            Extent2D size, MeshRes mesh, ModelRes model);

    void initSceneState(flecs::entity e);
}