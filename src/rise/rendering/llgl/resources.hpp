#pragma once
#include <SDL.h>
#include <LLGL/LLGL.h>
#include <flecs.h>
#include "pipelines/scene.hpp"
#include "util/soa.hpp"

namespace rise::rendering {
    // id + version
    using Key = std::pair<unsigned, unsigned>;

    struct TextureId {
        Key id;
    };

    struct TextureState {
        LLGL::Texture* val = nullptr;
    };

    void importTexture(flecs::world& ecs);

    struct MeshId {
        Key id;
    };

    struct MeshState {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };

    void importMesh(flecs::world& ecs);

    struct MaterialId {
        Key id;
    };

    struct MaterialState {
        LLGL::Buffer *uniform = nullptr;
    };

    void importMaterial(flecs::world& ecs);

    struct ModelId {
        Key id;
    };

    struct ModelState {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    void importModel(flecs::world& ecs);

    struct ViewportId {
        Key id;
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

    void importViewport(flecs::world& ecs);

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
        LLGL::PipelineLayout* layout = nullptr;
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::VertexFormat format;
        LLGL::ResourceHeap* heap = nullptr;
        LLGL::Buffer* uniform = nullptr;
        MeshState mesh;
    };

    struct Presets {
        flecs::entity material;
        flecs::entity mesh;
        flecs::entity texture;
    };

    struct Manager {
        SoaSlotMap<TextureState> textures;
        std::vector<std::map<TextureState, TextureId>> texturesToInit;
        std::vector<TextureId> texturesToRemove;

    };

    struct ApplicationState {
        CoreState core;
        Manager manager;
        SceneState scene;
        GuiState gui;
        Presets presets;
    };

    struct ApplicationId {
        ApplicationState* id;
    };

    struct ApplicationRef {
        flecs::ref<ApplicationId> ref;
    };

    void importApplication(flecs::world& ecs);
}
