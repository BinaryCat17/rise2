#include "module.hpp"
#include "resources.hpp"
#include "pipeline.hpp"
#include "platform.hpp"
#include "input.hpp"
#include "debug.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
    struct ModelRes {
        LLGL::Buffer *cameraBuffer = nullptr;
        LLGL::Buffer *objectBuffer = nullptr;
        LLGL::Buffer *materialBuffer = nullptr;
        LLGL::Texture *texture = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    struct CameraRes {
        LLGL::Buffer *uniform = nullptr;
    };

    struct MaterialRes {
        LLGL::Buffer *uniform = nullptr;
    };

    struct Instance {
        std::vector<std::function<void(flecs::world &ecs)>> imguiCallbacks{};
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        PipelineData pipeline{};
        LLGL::Buffer *globalData = nullptr;
        LLGL::Sampler *sampler = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::CommandQueue *cmdQueue = nullptr;
        size_t lastLightId{};
        scenePipeline::Global *pGlobal{};
    };

    struct RenderParameters {
        std::string root = ".";
        std::string title = "rise";
        glm::vec2 windowSize{800, 600};
    };

    struct RenderStateT {
        std::string root;
        Instance instance;
        flecs::entity modelPrefab;
        flecs::entity cameraPrefab;
    };

    using RenderState = std::shared_ptr<RenderStateT>;

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    LLGL::ResourceHeap *makeResourceHeap(Instance &instance, ModelRes &model) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = instance.pipeline.layout;
        resourceHeapDesc.resourceViews.emplace_back(instance.globalData);
        resourceHeapDesc.resourceViews.emplace_back(model.cameraBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.materialBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.objectBuffer);
        resourceHeapDesc.resourceViews.emplace_back(instance.sampler);
        resourceHeapDesc.resourceViews.emplace_back(model.texture);
        return instance.renderer->CreateResourceHeap(resourceHeapDesc);
    }

    glm::vec3 calcOrigin(glm::vec3 position, glm::vec3 rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z));
        direction.y = std::sin(glm::radians(rotation.z));
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z));
        glm::vec3 cameraFront = glm::normalize(direction);
        return position + direction * 3.f;
    }

    void onLoad(flecs::world &ecs, Instance &instance) {
        ecs.system<>().kind(flecs::OnLoad).iter([&instance](flecs::iter) {
            instance.pGlobal = mapUniformBuffer<scenePipeline::Global>(
                    instance.renderer.get(), instance.globalData);
            instance.lastLightId = 0;
        });

        ecs.system<>(nullptr, "Material").kind(flecs::OnAdd).
                each([&instance](flecs::entity e) {
            e.set<MaterialRes>({createUniformBuffer(instance.renderer.get(), MaterialRes{})});
        });

        ecs.system<>(nullptr, "Viewport").kind(flecs::OnAdd).
                each([&instance](flecs::entity e) {
            e.set<CameraRes>({createUniformBuffer(instance.renderer.get(), CameraRes{})});
        });
    }

    void postLoad(flecs::world &ecs, Instance &instance) {
        ecs.system<const WorldPosition, const DiffuseColor, const Distance, const Intensity>().
                kind(flecs::PostLoad).each([&instance](flecs::entity e, WorldPosition position,
                DiffuseColor color, Distance distance, Intensity intensity) {
            if (instance.lastLightId < scenePipeline::maxLightCount) {
                auto &light = instance.pGlobal->pointLights[instance.lastLightId];
                light.diffuse = color.val;
                light.position = position.val;
                light.distance = distance.val;
                light.intensity = intensity.val;
                ++instance.lastLightId;
            }
        });

        ecs.system<const WorldPosition, const WorldRotation, CameraRes>().kind(flecs::OnSet).
                each([&instance](flecs::entity, WorldPosition position, WorldRotation rotation,
                CameraRes &camera) {
            glm::vec3 origin = calcOrigin(position.val, rotation.val);

            scenePipeline::PerCamera data;
            data.view = glm::lookAt(position.val, origin, glm::vec3(0, 1, 0));
            auto windowSize = instance.context->GetResolution();
            data.projection = glm::perspective(glm::radians(45.0f),
                    static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height),
                    0.1f, 100.0f);
            updateUniformBuffer(instance.renderer.get(), camera.uniform, data);
        });

        ecs.system<const TextureRes, const MaterialRes, ModelRes>().kind(flecs::PostLoad).
                each([&instance](flecs::entity, TextureRes texture, MaterialRes material,
                ModelRes &model) {
            model.texture = texture.texture;
            model.materialBuffer = material.uniform;
            model.heap = makeResourceHeap(instance, model);
        });

        ecs.system<const WorldPosition, const WorldRotation, const WorldScale, ModelRes>().
                kind(flecs::PostLoad).each([&instance](flecs::entity, WorldPosition position,
                WorldRotation rotation, WorldScale scale, ModelRes &model) {
            glm::mat4 mat = glm::translate(glm::mat4(1), position.val);
            float angle = std::max({rotation.val.x, rotation.val.y, rotation.val.z});
            mat = glm::rotate(mat, glm::radians(angle), glm::normalize(rotation.val));
            mat = glm::scale(mat, scale.val);

            updateUniformBuffer(instance.renderer.get(), model.objectBuffer,
                    scenePipeline::PerObject{mat});
        });

        ecs.system<const DiffuseColor, MaterialRes>().kind(flecs::PostLoad).
                each([&instance](flecs::entity, DiffuseColor color, MaterialRes &material) {
            scenePipeline::PerMaterial data;
            data.diffuseColor = glm::vec4(color.val, 1.0f);
            updateUniformBuffer(instance.renderer.get(), material.uniform, data);
        });
    }

    void preUpdate(flecs::world &ecs, Instance &instance) {
        ecs.system<>().kind(flecs::PreUpdate).iter([&instance](flecs::iter) {
            for (; instance.lastLightId < scenePipeline::maxLightCount; ++instance.lastLightId) {
                instance.pGlobal->pointLights[instance.lastLightId].diffuse = glm::vec3(0, 0, 0);
            }

            instance.renderer->UnmapBuffer(*instance.globalData);

            auto cmdBuffer = instance.cmdBuf;
            cmdBuffer->Begin();
            cmdBuffer->BeginRenderPass(*instance.context);

            cmdBuffer->SetPipelineState(*instance.pipeline.pipeline);
            cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);
        });
    }

    void onUpdate(flecs::world &ecs, Instance &instance) {
        ecs.system<const ModelRes, const MeshRes>().kind(flecs::OnUpdate).
                iter([&instance](flecs::iter it, ModelRes const *model, MeshRes const *mesh) {
            flecs::column<const Viewport> parent(it, 3);
            LLGL::Viewport viewport{parent->pos.x, parent->pos.y, parent->size.x, parent->size.y};

            auto resolution = instance.context->GetResolution();
            assert(viewport.x + viewport.width <= resolution.width ||
                    viewport.y + viewport.height <= resolution.height);
            instance.cmdBuf->SetViewport(resolution);

            for (auto i : it) {
                auto pModel = getFromIt(it, i, model);
                auto pMesh = getFromIt(it, i, mesh);
                instance.cmdBuf->SetResourceHeap(*pModel->heap);
                instance.cmdBuf->SetVertexBuffer(*pMesh->vertices);
                instance.cmdBuf->SetIndexBuffer(*pMesh->indices);
                instance.cmdBuf->DrawIndexed(pMesh->numIndices, 0);
            }
        });
    }

    void postUpdate(flecs::world &ecs, Instance &instance) {
        ecs.system<>().kind(flecs::PreUpdate).iter([&instance](flecs::iter) {
            instance.cmdBuf->EndRenderPass();
            instance.cmdBuf->End();

            instance.cmdQueue->Submit(*instance.cmdBuf);
            instance.context->Present();
        });
    }

    Instance createInstance(std::string const &root, SDL_Window *window) {
        Instance instance;

        instance.renderer = createRenderer();
        auto renderer = instance.renderer.get();

        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        instance.context = createRenderingContext(renderer, window);
        instance.window = window;
        instance.pipeline = scenePipeline::make(renderer, root);
        instance.globalData = createUniformBuffer(renderer, scenePipeline::Global{});
        instance.sampler = makeSampler(renderer);
        instance.cmdQueue = instance.renderer->GetCommandQueue();
        instance.cmdBuf = instance.renderer->CreateCommandBuffer();

        return instance;
    }

    RenderModule::RenderModule(flecs::world &ecs) {
        ecs.module<RenderModule>();
        ecs.component<Viewport>("Viewport");
        ecs.component<WorldPosition>("WorldPosition");
        ecs.component<WorldRotation>("WorldRotation");
        ecs.component<WorldScale>("WorldScale");
        ecs.component<DiffuseColor>("DiffuseColor");
        ecs.component<Distance>("Distance");
        ecs.component<Intensity>("Intensity");
        ecs.component<Path>("Path");
        ecs.component<Material>("Material");
        ecs.component<PointLight>("PointLight");
        ecs.component<Texture>("Texture");
        ecs.component<Mesh>("Mesh");
        ecs.component<MeshRes>("MeshRes");
        ecs.component<TextureRes>("TextureRes");
        ecs.component<MaterialRes>("MaterialRes");
        ecs.component<CameraRes>("CameraRes");

        auto params = getOrDefault<RenderParameters>(ecs);

        auto state = std::make_shared<RenderStateT>(RenderStateT{
                params->root,
                createInstance(params->root, createGameWindow(params->title, getWindowSize(ecs))),
                ecs.prefab("ModelBase")
                        .set<WorldPosition>({glm::vec3{0.f, 0.f, 0.f}})
                        .set<WorldRotation>({glm::vec3(0.f, 0.f, 0.f)})
                        .set<WorldScale>({glm::vec3(1.f, 1.f, 1.f)}),
                ecs.prefab("CameraBase")
                        .set<WorldPosition>({glm::vec3{0.f, 0.f, 0.f}})
                        .set<WorldRotation>({glm::vec3(0.f, 0.f, 0.f)})
        });

        auto &instance = state->instance;

        ecs.set<RenderState>(state);

        RenderModule::loadMesh(ecs, state->modelPrefab, "cube.obj");
        RenderModule::loadTexture(ecs, state->modelPrefab, "default.jpg");
        state->modelPrefab.set<MaterialRes>({
                createUniformBuffer(instance.renderer.get(), scenePipeline::PerMaterial{})
        });
        state->cameraPrefab.set<CameraRes>({
                createUniformBuffer(instance.renderer.get(), scenePipeline::PerCamera{})
        });
        onLoad(ecs, state->instance);
        postLoad(ecs, state->instance);
        preUpdate(ecs, state->instance);
        onUpdate(ecs, state->instance);
    }

    void RenderModule::loadTexture(flecs::world &ecs, flecs::entity e, const std::string &path) {
        auto state = checkGet<RenderState>(ecs);
        e.set<TextureRes>(loadTextureFromDisk(state->instance.renderer.get(),
                state->root + "/textures/" + path));
        e.add<Texture>();
    }

    void RenderModule::loadMesh(flecs::world &ecs, flecs::entity e, const std::string &path) {
        auto state = checkGet<RenderState>(ecs);
        e.set<MeshRes>(loadMeshFromDisk(state->instance.renderer.get(),
                state->root + "/models/" + path, state->instance.pipeline.format));
        e.add<Mesh>();
    }

    void RenderModule::renderTo(flecs::world &ecs, flecs::entity e, flecs::entity viewport) {
        auto state = checkGet<RenderState>(ecs);

        if (!e.has_instanceof(state->modelPrefab)) {
            e.add_instanceof(state->modelPrefab);
        }
        if (!viewport.has_instanceof(state->cameraPrefab)) {
            viewport.add_instanceof(state->cameraPrefab);
        }

        e.add_childof(viewport);

        auto &model = checkGet<ModelRes>(ecs);
        model.cameraBuffer = checkGet<CameraRes>(viewport).uniform;
        model.materialBuffer = checkGet<MaterialRes>(e).uniform;
        model.texture = checkGet<TextureRes>(e).texture;
        model.objectBuffer = createUniformBuffer(state->instance.renderer.get(),
                scenePipeline::PerObject{});
        model.heap = makeResourceHeap(state->instance, model);
    }

    void RenderModule::setWindowSize(flecs::world &ecs, glm::vec2 size) {
        if (auto state = ecs.get<RenderState>()) {
            SDL_SetWindowSize((*state)->instance.window, static_cast<int>(size.x),
                    static_cast<int>(size.y));
        } else {
            getOrSet<RenderParameters>(ecs).windowSize = size;
        }
    }

    glm::vec2 RenderModule::getWindowSize(flecs::world &ecs) {
        if (auto state = ecs.get<RenderState>()) {
            int width, height;
            SDL_GetWindowSize((*state)->instance.window, &width, &height);
            return {static_cast<float>(width), static_cast<float>(height)};
        } else {
            return getOrSet<RenderParameters>(ecs).windowSize;
        }
    }

    void RenderModule::setWorkDirectory(flecs::world &ecs, const std::string &dir) {
        getOrSet<RenderParameters>(ecs).root = dir;
    }

    void RenderModule::setWindowTitle(flecs::world &ecs, const std::string &dir) {
        if (auto state = ecs.get_mut<RenderState>()) {
            SDL_SetWindowTitle((*state)->instance.window, (*state)->root.c_str());
        } else {
            getOrSet<RenderParameters>(ecs).title = dir;
        }
    }

    std::string RenderModule::getWindowTitle(flecs::world &ecs) {
        if (auto state = ecs.get_mut<RenderState>()) {
            return SDL_GetWindowTitle((*state)->instance.window);
        } else {
            return getOrSet<RenderParameters>(ecs).title;
        }
    }

    void RenderModule::gui(flecs::world &ecs, std::function<void()>) {}
}
