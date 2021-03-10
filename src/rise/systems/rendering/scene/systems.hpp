#pragma once

#include "../core/resources.hpp"
#include "components/rendering/module.hpp"
#include "mesh.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateResourceHeap(flecs::entity, RenderSystem const &renderer, PipelineLayout layout,
            ResourceHeap &heap, DiffuseTextureRes diffuse, MaterialRes material,
            TransformRes transform, ViewportRes viewport, Sampler sampler);

    void updateTransform(flecs::entity, RenderSystem const &renderer, TransformRes transform,
            Position3D position, Rotation3D rotation, Scale3D scale);

    void updateMaterial(flecs::entity, RenderSystem const &renderer, MaterialRes material,
            DiffuseColor color);

    void updateViewport(flecs::entity, RenderSystem const &renderer, ViewportRes const &viewport,
            Extent2D size, Position3D position, Rotation3D rotation);

    void renderScene(flecs::entity, RenderSystem const &renderer);
}