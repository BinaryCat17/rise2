#pragma once

#include "../core/resources.hpp"
#include "components/rendering/module.hpp"
#include "mesh.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateResourceHeap(flecs::entity, RenderSystem &renderer, PipelineLayout layout,
            ResourceHeap &heap, DiffuseTextureRes diffuse, MaterialRes material,
            TransformRes transform, ViewportRes viewport, Sampler sampler);

    void updateTransform(flecs::entity, RenderSystem &renderer, TransformRes transform,
            Position3D position, Rotation3D rotation, Scale3D scale);

    void updateMaterial(flecs::entity, RenderSystem &renderer, MaterialRes material,
            DiffuseColor color);

    void updateViewport(flecs::entity, RenderSystem &renderer, ViewportRes &viewport,
            Extent2D size, Position3D position, Rotation3D rotation);

    void renderScene(flecs::entity, RenderSystem &renderer, CommandBuffer cmdBuf,
            Pipeline pipeline, Extent2D resolution);
}