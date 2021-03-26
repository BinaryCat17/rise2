#include "viewport.hpp"
#include "utils.hpp"
#include "math.hpp"
#include "../glm.hpp"

namespace rise::rendering {
    void regViewport(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
        if (!e.has<Extent2D>()) e.set<Extent2D>({0.0f, 0.0f});

    }

    void initViewport(flecs::entity e, ApplicationRef app, ViewportId& id) {
        if(!e.owns<Initialized>()) {
            auto &core = app.ref->id->core;
            auto uniform = createUniformBuffer<scenePipeline::PerViewport>(core.renderer.get());

            std::tuple init{
                    ViewportState{uniform},
                    UpdatedViewportState{},
                    std::vector<flecs::entity>{}
            };

            id.id = getApp(e)->manager.viewport.states.push_back(std::move(init));
            e.set<ViewportRef>({e.get_ref<ViewportId>()});
            e.add<Initialized>();
        }
    }

    void regPointLight(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0f, 1.0f, 1.0f});
        if (!e.has<Intensity>()) e.set<Intensity>({1.0f});
        if (!e.has<Distance>()) e.set<Distance>({15.f});
    }

    void unregViewport(flecs::entity e) {
        if(e.owns<Initialized>()) {
            getApp(e)->manager.viewport.toRemove.push_back(*e.get<ViewportId>());
            e.remove<ViewportId>();
            e.remove<Initialized>();
        }
    }

    void catchCameraUpdate(flecs::entity, ApplicationRef app, ViewportId viewport) {
        auto &manager = app.ref->id->manager;
        std::get<eViewportUpdated>(manager.viewport.states.at(viewport.id)).get().camera = true;
    }

    void catchLightUpdate(flecs::entity, ApplicationRef app, ViewportId viewport) {
        auto &manager = app.ref->id->manager;
        std::get<eViewportUpdated>(manager.viewport.states.at(viewport.id)).get().light = true;
    }

    void prepareViewport(flecs::entity, ApplicationRef ref, ViewportId viewportId) {
        auto &&row = ref.ref->id->manager.viewport.states.at(viewportId.id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto updated = std::get<eViewportUpdated>(row).get();

        if (updated.camera || updated.light) {
            viewport.pData = mapUniformBuffer<scenePipeline::PerViewport>(
                    ref.ref->id->core.renderer.get(), viewport.uniform);
        }
    }

    void updateViewportCamera(flecs::entity, ApplicationRef ref, ViewportId viewportId,
            Extent2D size, Position3D position, Rotation3D rotation) {
        auto &&row = ref.ref->id->manager.viewport.states.at(viewportId.id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto updated = std::get<eViewportUpdated>(row).get();

        if (updated.camera) {
            if (size.width != 0 && size.height != 0) {
                glm::vec3 origin = calcCameraOrigin(toGlm(position), toGlm(rotation));
                viewport.pData->view = glm::lookAt(toGlm(position), origin, glm::vec3(0, 1, 0));
                viewport.pData->projection = glm::perspective(glm::radians(45.0f),
                        size.width / size.height, 0.1f, 100.0f);
            } else {
                std::cerr << "extent must not be null" << std::endl;
            }
        }
    }

    void updateViewportLight(flecs::entity, ApplicationRef ref, ViewportRef viewportRef,
            Position3D position, DiffuseColor color, Intensity intensity, Distance distance) {
        auto &&row = ref.ref->id->manager.viewport.states.at(viewportRef.ref->id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto updated = std::get<eViewportUpdated>(row).get();

        if (updated.currentLight < scenePipeline::maxLightCount) {
            auto &light = viewport.pData->pointLights[updated.currentLight];
            light.position = toGlm(position);
            light.diffuse = toGlm(color);
            light.intensity = intensity.factor;
            light.distance = distance.meters;
            ++updated.currentLight;
        }
    }

    void finishViewport(flecs::entity, ApplicationRef ref, ViewportId viewportId) {
        auto &&row = ref.ref->id->manager.viewport.states.at(viewportId.id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto updated = std::get<eViewportUpdated>(row).get();

        for (; updated.currentLight != scenePipeline::maxLightCount; ++updated.currentLight) {
            viewport.pData->pointLights[updated.currentLight].intensity = 0;
        }

        ref.ref->id->core.renderer->UnmapBuffer(*viewport.uniform);
        viewport.pData = nullptr;
        updated.currentLight = 0;
        updated.camera = false;
        updated.light = false;
    }

    void importViewport(flecs::world &ecs) {
        ecs.system<>("regViewport", "Viewport").kind(flecs::OnAdd).each(regViewport);
        ecs.system<>("regPointLight", "PointLight").kind(flecs::OnAdd).each(regPointLight);
        ecs.system<>("unregViewport", "Viewport").kind(flecs::OnRemove).each(unregViewport);
        ecs.system<const ApplicationRef, ViewportId>("initViewport", "!Initialized").
                kind(flecs::OnSet).each(initViewport);
        ecs.system<ApplicationRef, ViewportId>("catchCameraUpdate",
                "Viewport, Initialized,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Extent2D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(catchCameraUpdate);

        ecs.system<ApplicationRef, ViewportId>("catchViewportUpdate",
                "Viewport, Initialized,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.DiffuseColor,"
                "[in] ANY:rise.rendering.Distance,"
                "[in] ANY:rise.rendering.Intensity").
                kind(flecs::OnSet).each(catchLightUpdate);
    }
}
