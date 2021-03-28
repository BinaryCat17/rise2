#include "shadows.hpp"
#include "utils.hpp"
#include "../glm.hpp"

namespace rise::rendering {
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
            textureDesc.arrayLayers = 6 * scenePipeline::maxLightCount;
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

    void regPointLight(flecs::entity e) {
        if (!e.has<Position3D>()) e.set<Position3D>({0.0f, 0.0f, 0.0f});
        if (!e.has<DiffuseColor>()) e.set<DiffuseColor>({1.0f, 1.0f, 1.0f});
        if (!e.has<Intensity>()) e.set<Intensity>({1.0f});
        if (!e.has<Distance>()) e.set<Distance>({15.f});
        e.set<LightId>({});
    }

    void initShadowModels(flecs::entity, ApplicationId app) {
        auto &shadows = app.id->shadows;
        auto &manager = app.id->manager;

        for (auto model : manager.light.toInitShadowModels) {
            for (auto &&light : manager.light.states) {
                auto &shadowModel = std::get<eLightShadowModels>(light).get();
                auto &lightState = std::get<eLightState>(light).get();
                LLGL::ResourceHeapDescriptor resourceHeapDesc;
                resourceHeapDesc.pipelineLayout = app.id->shadows.layout;
                resourceHeapDesc.resourceViews.emplace_back(lightState.uniform);
                resourceHeapDesc.resourceViews.emplace_back(
                        std::get<eModelState>(manager.model.states.at(model.id)).get().uniform);
                shadowModel.at(model.id).heap =
                        app.id->core.renderer->CreateResourceHeap(resourceHeapDesc);
            }
        }
    }

    void removeShadowModels(flecs::entity, ApplicationId app) {
        auto &manager = app.id->manager;
        for (auto model : manager.light.toRemoveShadowModels) {
            for (auto &&light : manager.light.states) {
                auto &shadowModel = std::get<eLightShadowModels>(light).get();
                app.id->core.renderer->Release(*shadowModel.at(model.id).heap);
                shadowModel.erase(model.id);
            }
        }
    }

    void initPointLight(flecs::entity e, ApplicationRef app, LightId &id) {
        if (id.id == NullKey) {
            auto &core = app.ref->id->core;
            auto &shadows = app.ref->id->shadows;

            std::tuple init{LightState{}, std::map<Key, ShadowModel>{}};

            id.id = app.ref->id->manager.light.states.push_back(std::move(init));
            e.add_trait<Initialized, LightId>();
            auto renderer = core.renderer.get();
            auto uniform = createUniformBuffer<shadowPipeline::PerLight>(renderer);
            app.ref->id->manager.light.toInit.emplace_back(LightState{uniform, 0}, id);
            app.ref->id->manager.light.toUpdate.push_back(e);

            stdext::slot_map<int>().z
            for(app.ref->id->manager.model.states.) {

            }

        } else {
            app.ref->id->manager.light.toRemove.push_back(id);
        }
    }

    void removePointLight(flecs::entity, ApplicationRef app, LightId id, ViewportRef viewport) {
        auto &manager = app.ref->id->manager;
        std::get<eViewportUpdated>(manager.viewport.states.at(viewport.ref->id)).get().light = true;
        app.ref->id->manager.light.toRemove.push_back(id);
    }

    void unregPointLight(flecs::entity e) {
        if (e.has_trait<Initialized, LightId>()) {
            e.remove<LightId>();
            e.remove_trait<Initialized, LightId>();
        }
    }

    void updateLightMatrices(flecs::entity, ApplicationRef ref, LightId light,
            Position3D position) {
        auto &&viewport = std::get<eLightState>(
                ref.ref->id->manager.light.states.at(light.id)).get();
        auto data = mapUniformBuffer<shadowPipeline::PerLight>(
                ref.ref->id->core.renderer.get(), viewport.uniform);

        float aspect = (float) shadowPipeline::resolution.width /
                (float) shadowPipeline::resolution.height;
        float near = 1.0f;
        float far = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
        auto lightPos = toGlm(position);
        auto &transforms = data->lightSpaceMatrix;

        transforms[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0),
                glm::vec3(0.0, -1.0, 0.0));
        transforms[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0),
                glm::vec3(0.0, -1.0, 0.0));
        transforms[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0),
                glm::vec3(0.0, 0.0, 1.0));
        transforms[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0),
                glm::vec3(0.0, 0.0, -1.0));
        transforms[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0),
                glm::vec3(0.0, -1.0, 0.0));
        transforms[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0),
                glm::vec3(0.0, -1.0, 0.0));
    }

    void prepareShadowPass(flecs::entity, ApplicationId app, size_t lightId) {
        auto &core = app.id->core;
        auto cmd = core.cmdBuf;
        cmd->BeginRenderPass(app.id->shadows.cubeTarget[lightId]);
        cmd->SetViewport(LLGL::Viewport(shadowPipeline::resolution));
        cmd->Clear(LLGL::ClearFlags::Depth);
    }

    void endShadowPass(flecs::entity, ApplicationId app) {
        auto &core = app.id->core;
        core.cmdBuf->EndRenderPass();
    }

    void importShadowsState(flecs::world &ecs) {
        ecs.system<>("regPointLight", "PointLight").kind(flecs::OnAdd).each(regPointLight);
        ecs.system<const ApplicationRef, const ViewportRef>("removePointLight", "PointLight").
                kind(EcsUnSet).each(removePointLight);
    }

    void initShadowsState(flecs::entity, ApplicationState &state) {
        state.shadows.cubeMaps = createDepthTexture(state.core.renderer.get());
        for (size_t i = 0; i != scenePipeline::maxLightCount; ++i) {
            state.shadows.cubeTarget[i] = createDepthTarget(state.core.renderer.get(),
                    state.shadows.cubeMaps, i * 6);
        }
    }
}
