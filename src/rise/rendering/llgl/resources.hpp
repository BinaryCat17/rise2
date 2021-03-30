#pragma once

#include <SDL.h>
#include <LLGL/LLGL.h>
#include <flecs.h>
#include <set>
#include "../module.hpp"
#include "pipelines.hpp"
#include "util/soa.hpp"

namespace rise::rendering {
    struct Previous {
        flecs::entity e;
    };

    struct TextureId {
        Key id = NullKey;
    };

    struct TextureState {
        LLGL::Texture *val = nullptr;
    };

    struct MeshId {
        Key id = NullKey;
    };

    struct MeshState {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };

    struct MaterialId {
        Key id = NullKey;
    };

    struct MaterialState {
        LLGL::Buffer *uniform = nullptr;
    };

    struct ModelId {
        Key id = NullKey;
    };

    struct ModelState {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    struct ViewportId {
        Key id = NullKey;
    };

    struct ViewportRef {
        flecs::ref<ViewportId> ref;
    };

    struct ShadowTarget {
        LLGL::RenderTarget *target = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
    };

    struct ViewportState {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        LLGL::Texture *cubeMaps = nullptr;
        std::array<ShadowTarget, scenePipeline::maxLightCount> cubeTarget{};
    };

    struct UpdatedViewportState {
        bool camera = true;
        bool light = true;
        size_t currentLight = 0;
    };

    inline ViewportId getViewport(flecs::entity e) {
        auto ref = e.get<ViewportRef>()->ref;
        return {ref->id};
    }

    struct LightState {
        LLGL::Buffer *matrices = nullptr;
        LLGL::Buffer *parameters = nullptr;
        size_t id = 0;
    };

    struct LightId {
        Key id = NullKey;
    };

    struct ShadowModel {
        LLGL::ResourceHeap *heap = nullptr;
    };

    struct CoreState {
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
    };

    struct Platform {
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
    };

    struct SceneState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
    };

    struct ShadowState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::ShaderProgram *program = nullptr;
        LLGL::VertexFormat format;
    };

    struct GuiState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
        LLGL::ResourceHeap *heap = nullptr;
        LLGL::Buffer *uniform = nullptr;
        MeshState mesh;
    };

    struct Presets {
        flecs::entity material;
        flecs::entity mesh;
        flecs::entity texture;
    };

    enum TextureSlots : int {
        eTextureState,
        eTextureModels
    };

    struct TextureResources {
        SoaSlotMap<TextureState, std::set<flecs::entity_t>> states;
        std::vector<std::pair<TextureState, TextureId>> toInit;
        std::vector<TextureId> toRemove;
    };

    enum ViewportSlots : int {
        eViewportState,
        eViewportUpdated,
        eViewportModels,
    };

    struct ViewportResources {
        SoaSlotMap<ViewportState, UpdatedViewportState, std::set<flecs::entity_t>> states;
        std::vector<std::pair<ViewportState, ViewportId>> toInit;
        std::vector<ViewportId> toRemove;
    };

    enum LightSlots : int {
        eLightState,
        eLightShadowModels,
    };

    struct LightResources {
        SoaSlotMap<LightState, std::map<Key, ShadowModel>> states;
        std::vector<std::pair<LightState, LightId>> toInit;
        std::vector<ModelId> toInitShadowModels;
        std::vector<ModelId> toRemoveShadowModels;
        std::vector<flecs::entity> toUpdate;
        std::vector<LightId> toRemove;
    };

    enum MaterialSlots : int {
        eMaterialState,
        eMaterialModels
    };

    struct MaterialResources {
        SoaSlotMap<MaterialState, std::set<flecs::entity_t>> states;
        std::vector<std::pair<MaterialState, MaterialId>> toInit;
        std::vector<flecs::entity> toUpdate;
        std::vector<MaterialId> toRemove;
    };

    enum ModelSlots : int {
        eModelState,
        eModelMeshes,
    };

    struct ModelResources {
        SoaSlotMap<ModelState, std::set<flecs::entity_t>> states;
        std::vector<std::pair<ModelState, ModelId>> toInit;
        std::vector<ModelId> toRemove;
        std::vector<flecs::entity_t> toUpdateDescriptors;
        std::vector<flecs::entity_t> toUpdateTransform;
    };

    enum MeshSlots : int {
        eMeshState,
    };

    struct MeshResources {
        SoaSlotMap<MeshState> states;
        std::vector<std::pair<MeshState, MeshId>> toInit;
        std::vector<MeshId> toRemove;
    };

    struct Manager {
        TextureResources texture;
        ViewportResources viewport;
        ModelResources model;
        MeshResources mesh;
        MaterialResources material;
        LightResources light;
    };

    template<auto n, typename T>
    void prepareRemove(Manager &manager, T &res) {
        // добавляем в очередь все модели для обновления наборов дескрипторов
        for (auto rm : res.toInit) {
            auto elem = res.states.at(rm.second.id);
            auto &models = std::get<n>(std::move(elem)).get();

            for (auto model : models) {
                manager.model.toUpdateDescriptors.push_back(model);
            }
        }

        for (auto rm : res.toRemove) {
            auto elem = res.states.at(rm.id);
            auto &models = std::get<n>(std::move(elem)).get();

            for (auto model : models) {
                manager.model.toUpdateDescriptors.push_back(model);
            }
        }
    }

    template<auto n, typename Res, typename Fn>
    void processRemoveInit(Manager &manager, Res &res, Fn &&f) {
        for (auto rm : res.toRemove) {
            auto elem = res.states.at(rm.id);
            auto &state = std::get<n>(std::move(elem)).get();

            f(state);

            manager.texture.states.erase(rm.id);
        }

        for (auto up : res.toInit) {
            auto elem = res.states.at(up.second.id);
            auto &state = std::get<n>(std::move(elem)).get();

            f(state);

            state = up.first;
        }
    }

    template<typename T>
    T const *getOrDefault(flecs::entity e, T init) {
        static T d{init};
        if (auto v = e.get<T>()) {
            return v;
        } else {
            return &d;
        }
    }

    struct ApplicationState {
        CoreState core;
        Manager manager;
        SceneState scene;
        GuiState gui;
        Platform platform;
        Presets presets;
        ShadowState shadows;
    };

    struct ApplicationId {
        ApplicationState *id = nullptr;
    };

    struct ApplicationRef {
        flecs::ref<ApplicationId> ref;
    };

    inline ApplicationState *getApp(flecs::entity e) {
        auto ref = e.get<ApplicationRef>()->ref;
        return ref->id;
    }

    struct Initialized {};
}
