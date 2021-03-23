#include "viewport.hpp"
#include "util/math.hpp"
#include "rendering/glm.hpp"
#include "../module.hpp"
#include "../core/state.hpp"
#include "../core/utils.hpp"

namespace rise::rendering {
    void regViewport(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
        if (!e.has<Extent2D>()) e.set<Extent2D>({0.0f, 0.0f});

        auto &core = *e.get<RegTo>()->e.get<CoreState>();
        auto &viewport = *e.get_mut<ViewportRes>();
        viewport.uniform = createUniformBuffer<scenePipeline::PerViewport>(core.renderer.get());
    }

    void regPointLight(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0f, 1.0f, 1.0f});
        if (!e.has<Intensity>()) e.set<Intensity>({1.0f});
        if (!e.has<Distance>()) e.set<Distance>({15.f});
    }

    void unregViewport(flecs::entity e) {
        auto &core = *e.get<CoreState>();
        auto &viewport = *e.get_mut<ViewportRes>();

        core.queue->WaitIdle();
        core.renderer->Release(*viewport.uniform);

        e.remove<ViewportRes>();
    }

    void makeCameraDirty(flecs::entity, ViewportRes &viewport) {
        viewport.dirtyCamera = true;
    }

    void prepareViewport(flecs::entity, RegTo state, ViewportRes &viewport) {
        auto renderer = state.e.get<CoreState>()->renderer.get();
        viewport.pData = mapUniformBuffer<scenePipeline::PerViewport>(renderer, viewport.uniform);
    }

    void updateViewportCamera(flecs::entity, ViewportRes &viewport, Extent2D size,
            Position3D position, Rotation3D rotation) {

        if (viewport.dirtyCamera) {
            if (size.width != 0 && size.height != 0) {
                glm::vec3 origin = calcCameraOrigin(toGlm(position), toGlm(rotation));
                auto data = viewport.pData;
                data->view = glm::lookAt(toGlm(position), origin, glm::vec3(0, 1, 0));
                data->projection = glm::perspective(glm::radians(45.0f),
                        size.width / size.height, 0.1f, 100.0f);
            } else {
                std::cerr << "extent must not be null" << std::endl;
            }
        }
    }

    void updateViewportLight(flecs::entity, ViewportRes &viewport, Position3D position,
            DiffuseColor color, Intensity intensity, Distance distance) {
        if (viewport.lightId < scenePipeline::maxLightCount) {
            auto &light = viewport.pData->pointLights[viewport.lightId];
            light.position = toGlm(position);
            light.diffuse = toGlm(color);
            light.intensity = intensity.factor;
            light.distance = distance.meters;
            ++viewport.lightId;
        }
    }

    void finishViewport(flecs::entity, RegTo state, ViewportRes &viewport) {
        for (; viewport.lightId != scenePipeline::maxLightCount; ++viewport.lightId) {
            viewport.pData->pointLights[viewport.lightId].intensity = 0;
        }

        auto &core = *state.e.get<CoreState>();
        core.renderer->UnmapBuffer(*viewport.uniform);
        viewport.pData = nullptr;
        viewport.lightId = 0;
        viewport.dirtyCamera = false;
    }

    void importViewport(flecs::world &ecs) {
        ecs.system<>("regViewport", "Viewport").kind(flecs::OnAdd).each(regViewport);
        ecs.system<>("regPointLight", "PointLight").kind(flecs::OnAdd).each(regPointLight);
        ecs.system<>("unregViewport", "Viewport").kind(flecs::OnRemove).each(unregViewport);

        ecs.system<ViewportRes>("makeCameraDirty",
                "Viewport,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Extent2D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(makeCameraDirty);

        ecs.system<const RegTo, ViewportRes>("prepareViewport", "Viewport").
                kind(flecs::PreStore).each(prepareViewport);

        ecs.system<ViewportRes, const Extent2D, const Position3D, const Rotation3D>(
                "updateViewportCamera", "Viewport").
                kind(flecs::PreStore).each(updateViewportCamera);

        ecs.system<ViewportRes, const Position3D, const DiffuseColor, const Intensity,
                const Distance>("updateViewportLight, rise.rendering.PointLight").
                kind(flecs::PreStore).each(updateViewportLight);

        ecs.system<const RegTo, ViewportRes>("finishViewport", "Viewport").
                kind(flecs::PreStore).each(finishViewport);
    }
}
