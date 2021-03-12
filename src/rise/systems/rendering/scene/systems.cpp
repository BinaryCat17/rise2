#include "systems.hpp"
#include "../core/utils.hpp"
#include "components/rendering/glm.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "pipeline.hpp"

namespace rise::systems::rendering {
    void updateResourceHeap(flecs::entity, RenderSystem &renderer, PipelineLayout layout,
            ResourceHeap &heap, DiffuseTextureRes diffuse, MaterialRes material,
            TransformRes transform, ViewportRes viewport, Sampler sampler) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = layout.val;
        resourceHeapDesc.resourceViews.emplace_back(viewport.val);
        resourceHeapDesc.resourceViews.emplace_back(material.val);
        resourceHeapDesc.resourceViews.emplace_back(transform.val);
        resourceHeapDesc.resourceViews.emplace_back(sampler.val);
        resourceHeapDesc.resourceViews.emplace_back(diffuse.e.get<Texture>()->val);

        if (heap.val) {
            renderer->Release(*heap.val);
        }

        heap.val = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void updateTransform(flecs::entity, RenderSystem &renderer, TransformRes transform,
            Position3D position, Rotation3D rotation, Scale3D scale) {
        glm::mat4 mat = glm::translate(glm::mat4(1), toGlm(position));
        float angle = std::max({rotation.x, rotation.y, rotation.z});
        mat = glm::rotate(mat, glm::radians(angle), glm::normalize(toGlm(rotation)));
        mat = glm::scale(mat, toGlm(scale));

        updateUniformBuffer(renderer.get(), transform.val, scenePipeline::PerObject{mat});
    }

    void updateMaterial(flecs::entity, RenderSystem &renderer, MaterialRes material,
            DiffuseColor color) {
        scenePipeline::PerMaterial data;
        data.diffuseColor = glm::vec4(color.r, color.g, color.b, 1);
        updateUniformBuffer(renderer.get(), material.val, data);
    }

    Position3D calcCameraOrigin(Position3D position, Rotation3D rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;
        direction.y = std::sin(glm::radians(rotation.z)) * 3;
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;

        return {position.x + direction.x, position.y + direction.y, position.z + direction.z};
    }

    void updateViewport(flecs::entity e, RenderSystem &renderer, ViewportRes &viewport,
            Extent2D size, Position3D position, Rotation3D rotation) {
        auto data = mapUniformBuffer<scenePipeline::PerViewport>(renderer.get(), viewport.val);

        Position3D origin = calcCameraOrigin(position, rotation);
        data->view = glm::lookAt(toGlm(position), toGlm(origin), glm::vec3(0, 1, 0));
        data->projection = glm::perspective(glm::radians(45.0f),
                size.width / size.height, 0.1f, 100.0f);

        size_t i = 0;
        for (auto child : e.children()) {
            for (auto row : child) {
                if (i < scenePipeline::maxLightCount) {
                    auto light = child.entity(row);
                    if (light.has<Intensity>()) {
                        data->pointLights[i].position = toGlm(*light.get<Position3D>());
                        data->pointLights[i].diffuse = toGlm(*light.get<DiffuseColor>());
                        data->pointLights[i].intensity = light.get<Intensity>()->factor;
                        data->pointLights[i].distance = light.get<Distance>()->meters;
                        ++i;
                    }
                }
            }
        }

        for (; i != scenePipeline::maxLightCount; ++i) {
            data->pointLights[i].intensity = 0;
        }

        renderer->UnmapBuffer(*viewport.val);
    }

    template<typename FnT>
    void forViewports(flecs::entity e, FnT &&f) {
        for (auto viewports : e.children()) {
            for (auto i : viewports) {
                auto viewportEntity = viewports.entity(i);
                auto pos = viewportEntity.get<Position2D>();
                auto extent = viewportEntity.get<Extent2D>();
                if (pos && extent) {
                    f(viewportEntity, *pos, *extent);
                }
            }
        }
    }

    template<typename FnT>
    void forModels(flecs::entity e, FnT &&f) {
        for (auto models : e.children()) {
            for (auto j : models) {
                auto modelEntity = models.entity(j);
                auto heap = modelEntity.get_mut<ResourceHeap>();
                auto mesh = modelEntity.get_mut<Mesh>();
                if (mesh && heap) {
                    f(modelEntity, *heap, *mesh);
                }
            }
        }
    }

    void renderScene(flecs::entity e, RenderSystem &renderer, CommandBuffer cmdBuf,
            Pipeline pipeline, Extent2D resolution) {
        cmdBuf.val->SetPipelineState(*pipeline.val);

        forViewports(e, [cmdBuf, resolution](flecs::entity ve,
                Position2D pos, Extent2D extent) {
            LLGL::Viewport viewport{pos.x, pos.y, extent.width, extent.height};

            assert(viewport.x + viewport.width <= resolution.width ||
                    viewport.y + viewport.height <= resolution.height);

            cmdBuf.val->SetViewport(viewport);

            forModels(ve, [cmdBuf](flecs::entity, ResourceHeap heap, Mesh mesh) {
                cmdBuf.val->SetResourceHeap(*heap.val);
                cmdBuf.val->SetVertexBuffer(*mesh.vertices);
                cmdBuf.val->SetIndexBuffer(*mesh.indices);
                cmdBuf.val->DrawIndexed(mesh.numIndices, 0);
            });
        });
    }
}
