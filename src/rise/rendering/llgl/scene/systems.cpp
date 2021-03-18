#include "systems.hpp"
#include "../core/utils.hpp"
#include "rendering/glm.hpp"

namespace rise::rendering {
    void updateResourceHeap(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            DiffuseTexture diffuse, MaterialRes material, ViewportRes viewport) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = scene.layout;
        resourceHeapDesc.resourceViews.emplace_back(viewport.uniform);
        resourceHeapDesc.resourceViews.emplace_back(material.uniform);
        resourceHeapDesc.resourceViews.emplace_back(model.uniform);
        resourceHeapDesc.resourceViews.emplace_back(core.sampler);
        resourceHeapDesc.resourceViews.emplace_back(diffuse.e.get<TextureRes>()->val);

        if (model.heap) {
            core.renderer->Release(*model.heap);
        }

        model.heap = core.renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void updateTransform(flecs::entity, CoreState &core, SceneState &scene, ModelRes &model,
            Position3D position, Rotation3D rotation, Scale3D scale) {
        glm::mat4 mat = glm::translate(glm::mat4(1), toGlm(position));
        float angle = std::max({rotation.x, rotation.y, rotation.z});
        if (angle != 0) {
            mat = glm::rotate(mat, glm::radians(angle), glm::normalize(toGlm(rotation)));
        }
        mat = glm::scale(mat, toGlm(scale));

        updateUniformBuffer(core.renderer.get(), model.uniform, scenePipeline::PerObject{mat});
    }

    void updateMaterial(flecs::entity, CoreState &core, SceneState &scene, MaterialRes material,
            DiffuseColor color) {
        scenePipeline::PerMaterial data;
        data.diffuseColor = glm::vec4(color.r, color.g, color.b, 1);
        updateUniformBuffer(core.renderer.get(), material.uniform, data);
    }

    Position3D calcCameraOrigin(Position3D position, Rotation3D rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;
        direction.y = std::sin(glm::radians(rotation.z)) * 3;
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;

        return {position.x + direction.x, position.y + direction.y, position.z + direction.z};
    }

    void dirtyViewportCamera(flecs::entity, ViewportRes &viewport) {
        viewport.dirtyCamera = true;
    }

    void dirtyViewportLight(flecs::entity, ViewportRes &viewport) {
        viewport.dirtyLight = true;
    }

    void updateRelative(flecs::entity, CoreState &core, Relative val) {
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(val.enabled));
    }

    void prepareViewport(flecs::entity, CoreState &core, ViewportRes &viewport) {
        if (viewport.dirtyCamera || viewport.dirtyLight) {
            viewport.lightId = 0;
            viewport.pData = mapUniformBuffer<scenePipeline::PerViewport>(core.renderer.get(),
                    viewport.uniform);
        }
    }

    void updateViewportCamera(flecs::entity, ViewportRes &viewport, Extent2D size,
            Position3D position, Rotation3D rotation) {
        if (viewport.dirtyCamera) {
            Position3D origin = calcCameraOrigin(position, rotation);
            auto data = viewport.pData;
            data->view = glm::lookAt(toGlm(position), toGlm(origin), glm::vec3(0, 1, 0));
            data->projection = glm::perspective(glm::radians(45.0f),
                    size.width / size.height, 0.1f, 100.0f);
        }
    }

    void updateViewportLight(flecs::entity, ViewportRes &viewport, Position3D position,
            DiffuseColor color, Intensity intensity, Distance distance) {
        if (viewport.dirtyLight && viewport.lightId < scenePipeline::maxLightCount) {
            auto &light = viewport.pData->pointLights[viewport.lightId];
            light.position = toGlm(position);
            light.diffuse = toGlm(color);
            light.intensity = intensity.factor;
            light.distance = distance.meters;
            ++viewport.lightId;
        }
    }

    void finishViewport(flecs::entity, CoreState &core, ViewportRes &viewport) {
        if (viewport.dirtyLight) {
            for (; viewport.lightId != scenePipeline::maxLightCount; ++viewport.lightId) {
                viewport.pData->pointLights[viewport.lightId].intensity = 0;
            }
        }

        if (viewport.dirtyCamera || viewport.dirtyLight) {
            core.renderer->UnmapBuffer(*viewport.uniform);
            viewport.dirtyLight = false;
            viewport.dirtyCamera = false;
        }
    }

    void renderScene(flecs::entity, CoreState &core, SceneState &scene, Position2D position,
            Extent2D size, MeshRes mesh, ModelRes model) {

        auto cmdBuf = core.cmdBuf;
        cmdBuf->SetPipelineState(*scene.pipeline);

        LLGL::Viewport viewport{position.x, position.y, size.width, size.height};
        cmdBuf->SetViewport(viewport);

        cmdBuf->SetResourceHeap(*model.heap);
        cmdBuf->SetVertexBuffer(*mesh.vertices);
        cmdBuf->SetIndexBuffer(*mesh.indices);
        cmdBuf->DrawIndexed(mesh.numIndices, 0);
    }

    void initSceneState(flecs::entity e) {
        auto &core = *e.get<CoreState>();
        SceneState scene;
        scene.format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        scene.format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        scene.layout = scenePipeline::createLayout(core.renderer.get());
        auto program = createShaderProgram(core.renderer.get(),
                core.path + "/shaders/scene", scene.format);
        scene.pipeline = scenePipeline::createPipeline(core.renderer.get(), scene.layout, program);
        e.set<SceneState>(scene);
    }
}
