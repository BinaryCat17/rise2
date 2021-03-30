#include "viewport.hpp"
#include "utils.hpp"
#include "math.hpp"
#include "../glm.hpp"

namespace rise::rendering {
    void regViewport(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Rotation3D>()) e.set<Rotation3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<Position2D>()) e.set<Position2D>({0.0f, 0.0f});
        if (!e.has<Extent2D>()) e.set<Extent2D>({1600.0f, 1000.0f});
        e.set<ViewportId>({});
    }

    LLGL::Texture *createDepthTexture(LLGL::RenderSystem *renderer) {
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.type = LLGL::TextureType::TextureCubeArray;
            textureDesc.bindFlags =
                    LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            textureDesc.format = LLGL::Format::D32Float;
            textureDesc.extent.width = shadowPipeline::resolution.width;
            textureDesc.extent.height = shadowPipeline::resolution.height;
            textureDesc.extent.depth = 1;
            textureDesc.bindFlags = LLGL::BindFlags::Sampled |
                    LLGL::BindFlags::DepthStencilAttachment;
            textureDesc.arrayLayers = scenePipeline::maxLightCount * 6;
        }
        return renderer->CreateTexture(textureDesc);
    }

    LLGL::RenderTarget *createDepthTarget(LLGL::RenderSystem *renderer, LLGL::Texture *map,
            uint32_t layer) {
        LLGL::RenderTargetDescriptor renderTargetDesc;
        renderTargetDesc.resolution = shadowPipeline::resolution;
        renderTargetDesc.attachments = {
                LLGL::AttachmentDescriptor{LLGL::AttachmentType::Depth, map, 0, layer}
        };
        return renderer->CreateRenderTarget(renderTargetDesc);
    }

    void initViewport(flecs::entity e, ApplicationRef app, ViewportId &id) {
        if (!e.has_trait<Initialized, ViewportId>()) {
            auto &core = app.ref->id->core;
            auto &shadows = app.ref->id->shadows;
            ViewportState state;

            state.uniform = createUniformBuffer<scenePipeline::PerViewport>(core.renderer.get());
            state.cubeMaps = createDepthTexture(core.renderer.get());
            for (size_t i = 0; i != scenePipeline::maxLightCount; ++i) {
                auto &target = state.cubeTarget[i];
                target.target = createDepthTarget(core.renderer.get(),
                        state.cubeMaps, i * 6);
                auto pass = target.target->GetRenderPass();
                target.pipeline = shadowPipeline::createPipeline(core.renderer.get(),
                        shadows.layout, shadows.program, pass);
            }

            std::tuple init{
                    state,
                    UpdatedViewportState{},
                    std::set<flecs::entity_t>{}
            };

            id.id = getApp(e)->manager.viewport.states.push_back(std::move(init));
            e.set<ViewportRef>({e.get_ref<ViewportId>()});
            e.add_trait<Initialized, ViewportId>();
        }
    }

    void unregViewport(flecs::entity e) {
        if (e.has_trait<Initialized, ViewportId>()) {
            getApp(e)->manager.viewport.toRemove.push_back(*e.get<ViewportId>());
            e.remove<ViewportId>();
            e.remove_trait<Initialized, ViewportId>();
        }
    }

    void catchCameraUpdate(flecs::entity, ApplicationRef app, ViewportId viewport) {
        auto &manager = app.ref->id->manager;
        std::get<eViewportUpdated>(manager.viewport.states.at(viewport.id)).get().camera = true;
    }

    void catchLightUpdate(flecs::entity, ApplicationRef app, ViewportRef viewport) {
        auto &manager = app.ref->id->manager;
        std::get<eViewportUpdated>(manager.viewport.states.at(viewport.ref->id)).get().light = true;
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
            Position3D position, DiffuseColor color, Intensity intensity, Distance distance,
            LightId &lightId) {
        auto &manager = ref.ref->id->manager;
        auto &&row = manager.viewport.states.at(viewportRef.ref->id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto &updated = std::get<eViewportUpdated>(row).get();

        if (updated.currentLight < scenePipeline::maxLightCount) {
            auto &lightState = std::get<eLightState>(manager.light.states.at(lightId.id)).get();
            lightState.id = updated.currentLight++;
            auto &light = viewport.pData->pointLights[lightState.id];
            light.position = toGlm(position);
            light.diffuse = toGlm(color);
            light.distance = distance.meters;
            light.intensity = intensity.factor;
        }
    }

    void finishViewport(flecs::entity, ApplicationRef ref, ViewportId viewportId) {
        auto &&row = ref.ref->id->manager.viewport.states.at(viewportId.id);
        auto &viewport = std::get<eViewportState>(row).get();
        auto &updated = std::get<eViewportUpdated>(row).get();

        if (updated.currentLight) {
            for (; updated.currentLight != scenePipeline::maxLightCount; ++updated.currentLight) {
                viewport.pData->pointLights[updated.currentLight].intensity = 0;
            }
        }

        if (updated.currentLight || updated.camera) {
            ref.ref->id->core.renderer->UnmapBuffer(*viewport.uniform);
        }

        viewport.pData = nullptr;
        updated.currentLight = 0;
        updated.camera = false;
        updated.light = false;
    }

    void importViewport(flecs::world &ecs) {
        ecs.system<>("regViewport", "Viewport").kind(flecs::OnAdd).each(regViewport);

        ecs.system<>("unregViewport", "Viewport").kind(flecs::OnRemove).each(unregViewport);
        ecs.system<const ApplicationRef, ViewportId>("initViewport",
                "!TRAIT | Initialized > ViewportId").
                kind(flecs::OnSet).each(initViewport);
        ecs.system<ApplicationRef, ViewportId>("catchCameraUpdate",
                "Viewport, TRAIT | Initialized > ViewportId,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.Extent2D,"
                "[in] ANY:rise.rendering.Rotation3D").
                kind(flecs::OnSet).each(catchCameraUpdate);

        ecs.system<ApplicationRef, ViewportRef>("catchLightUpdate",
                "PointLight,"
                "[in] ANY:rise.rendering.Position3D,"
                "[in] ANY:rise.rendering.DiffuseColor,"
                "[in] ANY:rise.rendering.Distance,"
                "[in] ANY:rise.rendering.Intensity").
                kind(flecs::OnSet).each(catchLightUpdate);
    }
}
