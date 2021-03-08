#include "module.hpp"
#include "components/rendering/module.hpp"
#include "resources.hpp"
#include "pipeline.hpp"
#include "platform.hpp"
#include "debug.hpp"
#include "renderGui.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise::systems {
    struct Instance {
        std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
        SDL_Window *window = nullptr;
        LLGL::RenderContext *context = nullptr;
        LLGL::CommandQueue *queue = nullptr;
        LLGL::CommandBuffer *cmdBuf = nullptr;
        LLGL::Sampler *sampler = nullptr;
        PipelineData pipeline{};
    };

    // module state
    struct RenderStateT {
        std::string root;
        Instance instance;
        ResourcePresets prefabs;
    };

    using RenderState = std::shared_ptr<RenderStateT>;

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    void makeResourceHeap(Instance &instance, ModelRes &model) {
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = instance.pipeline.layout;
        resourceHeapDesc.resourceViews.emplace_back(model.viewportBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.materialBuffer);
        resourceHeapDesc.resourceViews.emplace_back(model.objectBuffer);
        resourceHeapDesc.resourceViews.emplace_back(instance.sampler);
        resourceHeapDesc.resourceViews.emplace_back(model.texture);

        if (model.heap) {
            instance.renderer->Release(*model.heap);
        }

        model.heap = instance.renderer->CreateResourceHeap(resourceHeapDesc);
    }

    glm::vec3 calcCameraOrigin(glm::vec3 position, glm::vec3 rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z));
        direction.y = std::sin(glm::radians(rotation.z));
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z));
        glm::vec3 cameraFront = glm::normalize(direction);
        return position + direction * 3.f;
    }

    // monitor resource changes
    void triggers(flecs::world &ecs, Instance &instance) {
        // makes resources dirty if they are updated

        ecs.system<ModelRes>("DirtyHeap", "[in] MaterialRes, [in] TextureRes").
                kind(flecs::OnSet).each([](flecs::entity, ModelRes &res) {
            res.dirtyHeap = true;
        });

        ecs.system<ModelRes>("DirtyModel",
                "[in] WorldPosition, [in] WorldRotation, [in] WorldScale").
                kind(flecs::OnSet).each([](flecs::entity, ModelRes &res) {
            res.dirtyModel = true;
        });

        ecs.system<ViewportRes>("DirtyCamera", "[in] WorldPosition, [in] WorldRotation").
                kind(flecs::OnSet).each([](flecs::entity, ViewportRes &res) {
            res.dirtyCamera = true;
        });

        ecs.system<>("DirtyLight",
                "[in] WorldPosition, [in] WorldRotation, [inout] PARENT:ViewportRes").
                kind(flecs::OnSet).iter([](flecs::iter it) {
            flecs::column<ViewportRes> parent(it, 3);
            parent->dirtyLight = true;
        });

        ecs.system<>("InitMaterial", "Material").kind(flecs::OnAdd).
                each([&instance](flecs::entity e) {
            e.set<MaterialRes>({createUniformBuffer(instance.renderer.get(), MaterialRes{})});
        });

        ecs.system<>("InitViewport", "Viewport").kind(flecs::OnAdd).
                each([&instance](flecs::entity e) {
            e.set<ViewportRes>({createUniformBuffer(instance.renderer.get(), ViewportRes{})});
        });
    }

    void onLoad(flecs::world &ecs, Instance &instance) {
        ecs.system<>().kind(flecs::OnLoad).iter([&ecs, &instance](flecs::iter) {
            if(!pullInputEvents(instance.window)) {
                ecs_quit(ecs.c_ptr());
            }
        });
    }

    // prepare and update all rendering resources
    void preStore(flecs::world &ecs, Instance &instance) {
        // update viewport resources

        ecs.system<ViewportRes>("PrepareViewport").kind(flecs::PreStore).
                each([&instance](flecs::entity, ViewportRes &res) {
            if (res.dirtyLight || res.dirtyCamera) {
                res.pData = mapUniformBuffer<scenePipeline::PerViewport>(
                        instance.renderer.get(), res.uniform);
                res.currentLightId = 0;
            }
        });

        ecs.system<const WorldPosition, const DiffuseColor, const Distance, const Intensity>(
                "UpdateViewportLight", "[inout] PARENT:ViewportRes").kind(flecs::PreStore).iter(
                [](flecs::iter it, WorldPosition const *positions,
                        DiffuseColor const *colors,
                        Distance const *distances, Intensity const *intensities) {
                    flecs::column<ViewportRes> parent(it, 5);
                    if (parent->dirtyLight) {
                        for (auto i : it) {
                            auto &light = parent->pData->pointLights[parent->currentLightId];
                            light.position = getFromIt(it, i, positions)->val;
                            light.diffuse = getFromIt(it, i, colors)->val;
                            light.distance = getFromIt(it, i, distances)->val;
                            light.intensity = getFromIt(it, i, intensities)->val;
                        }
                    }
                });

        ecs.system<const WorldPosition, const WorldRotation, ViewportRes>("UpdateViewportCamera").
                kind(flecs::PreStore).each([&instance](flecs::entity, WorldPosition position,
                WorldRotation rotation, ViewportRes &res) {
            if (res.dirtyCamera) {
                glm::vec3 origin = calcCameraOrigin(position.val, rotation.val);
                res.pData->view = glm::lookAt(position.val, origin, glm::vec3(0, 1, 0));

                auto windowSize = instance.context->GetResolution();
                res.pData->projection = glm::perspective(glm::radians(45.0f),
                        static_cast<float>(windowSize.width) /
                                static_cast<float>(windowSize.height),
                        0.1f, 100.0f);
            }
        });

        ecs.system<ViewportRes>("FinishViewport").kind(flecs::PreStore).
                each([&instance](flecs::entity, ViewportRes &res) {
            if (res.dirtyLight || res.dirtyCamera) {
                instance.renderer->UnmapBuffer(*res.uniform);
                res.currentLightId = 0;
                res.dirtyCamera = false;
                res.dirtyLight = false;
            }
        });

        // update model resources

        ecs.system<const TextureRes, const MaterialRes, ModelRes>("UpdateModelHeap").kind(
                flecs::PreStore).
                each([&instance](flecs::entity, TextureRes texture, MaterialRes material,
                ModelRes &res) {
            if (res.dirtyHeap) {
                res.texture = texture.texture;
                res.materialBuffer = material.uniform;
                makeResourceHeap(instance, res);
                res.dirtyHeap = false;
            }
        });

        ecs.system<const WorldPosition, const WorldRotation, const WorldScale, ModelRes>(
                "UpdateModelTransform").kind(flecs::PreStore).each([&instance](flecs::entity,
                WorldPosition position, WorldRotation rotation, WorldScale scale, ModelRes &res) {
            if (res.dirtyModel) {
                glm::mat4 mat = glm::translate(glm::mat4(1), position.val);
                float angle = std::max({rotation.val.x, rotation.val.y, rotation.val.z});
                mat = glm::rotate(mat, glm::radians(angle), glm::normalize(rotation.val));
                mat = glm::scale(mat, scale.val);

                updateUniformBuffer(instance.renderer.get(), res.objectBuffer,
                        scenePipeline::PerObject{mat});
                res.dirtyModel = false;
            }
        });

        // update material resources

        ecs.system<const DiffuseColor, MaterialRes>("UpdateMaterial").kind(flecs::PreStore).
                each([&instance](flecs::entity, DiffuseColor color, MaterialRes &material) {
            scenePipeline::PerMaterial data;
            data.diffuseColor = glm::vec4(color.val, 1.0f);
            updateUniformBuffer(instance.renderer.get(), material.uniform, data);
        });
    }

    void onStore(flecs::world &ecs, Instance &instance) {
        ecs.system<>("PrepareRender").kind(flecs::OnStore).iter([&instance](flecs::iter) {
            auto cmd = instance.cmdBuf;
            cmd->Begin();
            cmd->BeginRenderPass(*instance.context);
            cmd->SetPipelineState(*instance.pipeline.pipeline);
            cmd->Clear(LLGL::ClearFlags::ColorDepth);
        });

        ecs.system<const ModelRes, const MeshRes>("WriteRender", "[in] PARENT:Viewport").
                kind(flecs::OnStore).iter(
                [&instance](flecs::iter it, ModelRes const *model, MeshRes const *mesh) {
                    flecs::column<const Viewport> parent(it, 3);
                    LLGL::Viewport viewport{parent->pos.x, parent->pos.y, parent->size.x,
                            parent->size.y};

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

        ecs.system<>("SubmitRender").kind(flecs::OnStore).iter([&ecs, &instance](flecs::iter) {
            renderGui(ecs, instance.renderer.get(), instance.window, instance.cmdBuf);

            instance.cmdBuf->EndRenderPass();
            instance.cmdBuf->End();
            instance.queue->Submit(*instance.cmdBuf);
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
        instance.sampler = makeSampler(renderer);
        instance.queue = instance.renderer->GetCommandQueue();
        instance.cmdBuf = instance.renderer->CreateCommandBuffer();

        return instance;
    }

    void registerComponents(flecs::world &ecs) {
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
        ecs.component<ViewportRes>("ViewportRes");
    }

    RenderModule::RenderModule(flecs::world &ecs) {
        registerComponents(ecs);

        auto params = getOrDefault<RenderParameters>(ecs);
        auto instance = createInstance(params->root,
                createGameWindow(params->title, getWindowSize(ecs)));

        initGui(ecs, instance.renderer.get(), instance.window, params->root);

        auto modelPrefab = ecs.prefab("ModelPrefab")
                .set<WorldPosition>({glm::vec3{0.f, 0.f, 0.f}})
                .set<WorldRotation>({glm::vec3(0.f, 0.f, 0.f)})
                .set<WorldScale>({glm::vec3(1.f, 1.f, 1.f)});


        auto viewportPrefab = ecs.prefab("ViewportPrefab")
                .set<WorldPosition>({glm::vec3{0.f, 0.f, 0.f}})
                .set<WorldRotation>({glm::vec3(0.f, 0.f, 0.f)});
        viewportPrefab.set<ViewportRes>({
                createUniformBuffer(instance.renderer.get(), scenePipeline::PerViewport{})
        });

        auto materialPrefab = ecs.prefab("MaterialPrefab").set<DiffuseColor>({glm::vec3{1, 1, 1}});
        materialPrefab.set<MaterialRes>({
                createUniformBuffer(instance.renderer.get(), scenePipeline::PerMaterial{})
        });


        Prefabs prefabs{
                ecs.type("ModelBase").add_instanceof(modelPrefab).
                        add<WorldPosition>().add<WorldRotation>().add<WorldScale>().add<ModelRes>().
                        add<MeshRes>().add<TextureRes>(),
                ecs.type("ViewportBase").add_instanceof(viewportPrefab).
                        add<WorldRotation>().add<WorldRotation>().add<ViewportRes>(),
                ecs.type("MaterialBase").add_instanceof(materialPrefab).
                        add<DiffuseColor, MaterialRes>()
        };


        auto state = std::make_shared<RenderStateT>(RenderStateT{
                params->root,
                std::move(instance),
                prefabs
        });

        ecs.set<RenderState>(state);

        triggers(ecs, state->instance);
        onLoad(ecs, state->instance);
        preStore(ecs, state->instance);
        onStore(ecs, state->instance);

        RenderModule::loadMesh(ecs, modelPrefab, "cube.obj");
        RenderModule::loadTexture(ecs, modelPrefab, "default.jpg");
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

        if(e.has<PointLight>()) {

        } else {
            e.add(state->prefabs.model);
            e.add(state->prefabs.material);
            if(!viewport.has<ViewportRes>()) {
                viewport.add(state->prefabs.viewport);
            }

            e.add_childof(viewport);

            ModelRes model;
            model.viewportBuffer = checkGet<ViewportRes>(viewport).uniform;
            model.materialBuffer = checkGet<MaterialRes>(e).uniform;
            model.texture = checkGet<TextureRes>(e).texture;
            model.objectBuffer = createUniformBuffer(state->instance.renderer.get(),
                    scenePipeline::PerObject{});
            makeResourceHeap(state->instance, model);

            e.set<ModelRes>({model});
        }
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

    void RenderModule::gui(flecs::world &ecs, std::function<void()> const& f) {
        addCallback(ecs, f);
    }
}
