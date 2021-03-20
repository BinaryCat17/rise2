#include "viewport.hpp"
#include "util/math.hpp"
#include "rendering/glm.hpp"
#include "../module.hpp"
#include "../core/state.hpp"
#include "../core/utils.hpp"

namespace rise::rendering {
    void regViewport(flecs::entity e) {
        if (e.owns<Viewport>()) {
            if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
            if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
            if (!e.has<Extent2D>()) e.set<Extent2D>({0.0f, 0.0f});

            e.set<ViewportRes>({});
        }
    }

    void unregViewport(flecs::entity e) {
        if (e.owns<Viewport>()) {
            e.remove<ViewportRes>();
        }
    }

    void initViewport(flecs::entity, CoreState &core, ViewportRes &viewport) {
        viewport.uniform = createUniformBuffer<scenePipeline::PerViewport>(core.renderer.get());
    }

    void removeViewport(flecs::entity, CoreState &core, ViewportRes &viewport) {
        core.renderer->Release(*viewport.uniform);
    }

    struct DirtyCameraTag {};

    void makeCameraDirty(flecs::entity e) {
        e.add<DirtyCameraTag>();
    }

    struct DirtyLightTag {};

    void makeLightDirty(flecs::entity e) {
        e.add<DirtyLightTag>();
    }

    void prepareViewport(flecs::entity, CoreState &core, ViewportRes &viewport) {
        viewport.pData = mapUniformBuffer<scenePipeline::PerViewport>(core.renderer.get(),
                viewport.uniform);
    }

    void updateViewportCamera(flecs::entity, ViewportRes &viewport, Extent2D size,
            Position3D position, Rotation3D rotation) {
        glm::vec3 origin = calcCameraOrigin(toGlm(position), toGlm(rotation));
        auto data = viewport.pData;
        data->view = glm::lookAt(toGlm(position), origin, glm::vec3(0, 1, 0));
        data->projection = glm::perspective(glm::radians(45.0f),
                size.width / size.height, 0.1f, 100.0f);
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

    void finishViewport(flecs::entity e, CoreState &core, ViewportRes &viewport) {
        if (viewport.lightId > 0) {
            for (; viewport.lightId != scenePipeline::maxLightCount; ++viewport.lightId) {
                viewport.pData->pointLights[viewport.lightId].intensity = 0;
            }
        }

        core.renderer->UnmapBuffer(*viewport.uniform);
        viewport.pData = nullptr;
        viewport.lightId = 0;

        e.remove<DirtyLightTag>();
        e.remove<DirtyCameraTag>();
    }

    void importMaterial(flecs::world &ecs) {
        ecs.component<DirtyCameraTag>("DirtyCameraTag");
        ecs.component<DirtyLightTag>("DirtyLightTag");

        ecs.system<>("regViewport", "Viewport").kind(flecs::OnAdd).each(regViewport);
        ecs.system<>("unregViewport", "Viewport").kind(flecs::OnRemove).each(unregViewport);
        ecs.system<CoreState, ViewportRes>("initViewport", "OWNED:ViewportRes").
                kind(flecs::OnSet).each(initViewport);
        ecs.system<CoreState, ViewportRes>("removeViewport", "OWNED:ViewportRes").
                kind(EcsUnSet).each(removeViewport);

        ecs.system<>("makeCameraDirty",
                "OWNED:ViewportRes,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Extent2D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(makeCameraDirty);

        ecs.system<>("makeLightDirty",
                "OWNED:ViewportRes,"
                "[in] rise.rendering.Position3D,"
                "[in] rise.rendering.DiffuseColor,"
                "[in] rise.rendering.Intensity,"
                "[in] rise.rendering.Distance").
                kind(flecs::OnSet).each(makeLightDirty);

        ecs.system<CoreState, ViewportRes>("prepareViewport",
                "OWNED:ViewportRes, OWNED:DirtyCameraTag || OWNED:DirtyLightTag").
                kind(flecs::PreStore).each(prepareViewport);

        ecs.system<ViewportRes, const Extent2D, const Position3D, const Rotation3D>(
                "updateViewportCamera", "OWNED:ViewportRes, OWNED:DirtyCameraTag").
                kind(flecs::PreStore).each(updateViewportCamera);

        ecs.system<ViewportRes, const Position3D, const DiffuseColor, const Intensity,
                const Distance>("updateViewportLight",
                "OWNED:ViewportRes, rise.rendering.PointLight, OWNED:DirtyLightTag").
                kind(flecs::PreStore).each(updateViewportLight);

        ecs.system<CoreState, ViewportRes>("finishViewport",
                "OWNED:ViewportRes, OWNED:DirtyCameraTag || OWNED:DirtyLightTag").
                kind(flecs::PreStore).each(finishViewport);
    }
}
