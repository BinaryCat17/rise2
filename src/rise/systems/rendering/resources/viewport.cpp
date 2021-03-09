#include "viewport.hpp"
#include "application.hpp"
#include "systems/rendering/util/resources.hpp"
#include "components/rendering/glm.hpp"

namespace rise::systems::rendering {
    Position3D calcCameraOrigin(Position3D position, Rotation3D rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;
        direction.y = std::sin(glm::radians(rotation.z)) * 3;
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;

        return {position.x + direction.x, position.y + direction.y, position.z + direction.z};
    }

    void prepareViewportSystem(flecs::iter it, ViewportResource *viewports) {
        auto &state = it.column<ApplicationResource>(2)->state;

        for (auto i : it) {
            auto viewport = getFromIt(it, i, viewports);
            if (viewport->dirtyCamera || viewport->dirtyLight) {
                viewport->pData = mapUniformBuffer<scenePipeline::PerViewport>(state.renderer.get(),
                        viewport->uniform);
            }
        }
    }

    void updateCameraSystem(flecs::entity, ViewportResource &viewport, Position3D position,
            Rotation3D rotation, Extent2D viewportSize) {
        if (viewport.dirtyCamera) {
            Position3D origin = calcCameraOrigin(position, rotation);
            viewport.pData->view = glm::lookAt(toGlm(position), toGlm(origin), glm::vec3(0, 1, 0));
            viewport.pData->projection = glm::perspective(glm::radians(45.0f),
                    viewportSize.width / viewportSize.height, 0.1f, 100.0f);
        }
    }

    void updateLightSystem(flecs::entity, ViewportResource &viewport, Position3D pos,
            Distance distance, DiffuseColor color, Intensity intensity) {
        if(viewport.dirtyLight) {
            auto &light = viewport.pData->pointLights[viewport.currentLightId];
            light.position = toGlm(pos);
            light.diffuse = toGlm(color);
            light.distance = getFromIt(it, i, distances)->val;
            light.intensity = getFromIt(it, i, intensities)->val;
        }
    }

    void finishViewportSystem(flecs::iter it, ViewportResource *viewports) {
        auto &state = it.column<ApplicationResource>(2)->state;

        for (auto row : it) {
            auto viewport = getFromIt(it, row, viewports);
            if (viewport->dirtyCamera || viewport->dirtyLight) {
                for (size_t i = viewport->currentLightId; i != scenePipeline::maxLightCount; ++i) {
                    viewport->pData->pointLights[i].distance = 0;
                }
                viewport->currentLightId = 0;
                viewport->dirtyLight = false;
                viewport->dirtyCamera = false;

                state.renderer->UnmapBuffer(*viewport->uniform);
            }
        }
    }
}