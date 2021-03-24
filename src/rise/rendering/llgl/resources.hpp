#pragma once

#include <SDL.h>
#include <LLGL/LLGL.h>
#include <flecs.h>
#include "pipelines/scene.hpp"
#include "util/soa.hpp"

namespace rise::rendering {

    struct TextureId {
        Key id = NullKey;
    };

    struct TextureState {
        LLGL::Texture *val = nullptr;
    };

    void importTexture(flecs::world &ecs);

    struct MeshId {
        Key id = NullKey;
    };

    struct MeshState {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };

    void importMesh(flecs::world &ecs);

    struct MaterialId {
        Key id = NullKey;
    };

    struct MaterialState {
        LLGL::Buffer *uniform = nullptr;
    };

    void importMaterial(flecs::world &ecs);

    struct ModelId {
        Key id = NullKey;
    };

    struct ModelState {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    void importModel(flecs::world &ecs);

    struct ViewportId {
        Key id = NullKey;
    };

    struct ViewportRef {
        flecs::ref<ViewportId> ref;
    };

    struct ViewportState {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        unsigned lightId = 0;
        bool dirtyCamera = true;
    };

    inline ViewportId getViewport(flecs::entity e) {
        auto ref = e.get<ViewportRef>()->ref;
        return { ref->id };
    }

    void importViewport(flecs::world &ecs);

    struct CoreState {
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
    };

    struct SceneState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
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

    struct Manager {
        SoaSlotMap<TextureState, std::vector<ModelId>> textures;
        std::vector<std::pair<TextureState, TextureId>> texturesToInit;
        std::vector<TextureId> texturesToRemove;
        SoaSlotMap<ModelState> models;
        std::vector<std::pair<ModelState, ModelId>> modelsToInit;
        std::vector<ModelId> modelsToRemove;
        std::vector<ModelId> updatedModels;
    };

    struct ApplicationState {
        CoreState core;
        Manager manager;
        SceneState scene;
        GuiState gui;
        Presets presets;
    };


    struct ApplicationId {
        ApplicationState *id = nullptr;
    };

    struct ApplicationRef {
        flecs::ref<ApplicationId> ref;
    };

    inline ApplicationState* getApp(flecs::entity e) {
        auto ref = e.get<ApplicationRef>()->ref;
        return ref->id;
    }

    void importApplication(flecs::world &ecs);
}
