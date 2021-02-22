#include "rendering.hpp"
#include "pipeline.hpp"
#include "platform.hpp"
#include "resources.hpp"
#include "gui.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include <chrono>
#include <imgui.h>

namespace rise {
    LLGL::RenderContext *
    makeContext(LLGL::RenderSystem *renderer, SDL_Window *window, unsigned width, unsigned height) {
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = {width, height};
        contextDesc.videoMode.fullscreen = false;
        contextDesc.vsync.enabled = true;
        contextDesc.samples = 8;
        LLGL::RenderContext *context = renderer->CreateRenderContext(contextDesc,
                std::make_shared<Surface>(window));

        const auto &info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        return context;
    }

    LLGL::Texture *createTexture(Instance *instance, std::string path) {
        path = instance->root + "/textures/" + path;
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char *imageBuffer = stbi_load(path.c_str(),
                &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to load image from file: " + path);

        auto texture = makeTextureFromData(instance->renderer.get(),
                (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB),
                imageBuffer, texWidth, texHeight);

        stbi_image_free(imageBuffer);

        return texture;
    }

    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            std::cerr << "error: " << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            std::cerr << "warning: " << message.GetText() << std::endl;
        }
    };

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        LLGL::Log::SetReportCallbackStd(std::cerr);
        return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
    }

    Instance makeInstance(std::string const &root, unsigned width, unsigned height) {
        ImGui::CreateContext();
        Instance instance;

        instance.renderer = createRenderer();
        auto window = createGameWindow("minecraft 2", width, height);
        instance.context = makeContext(instance.renderer.get(), window, width, height);

        instance.window = window;

        instance.layout = makeLayout(instance.renderer.get());
        instance.program = makeProgram(instance.renderer.get(), root + "/shaders/diffuse");
        instance.pipeline = makePipeline(instance.renderer.get(), instance.layout,
                instance.program);

        impl::GlobalShaderData data{glm::mat4(1), glm::mat4(1)};
        instance.globalShaderData = createUniformBuffer(instance.renderer.get(), data);

        instance.root = root;
        instance.sampler = makeSampler(instance.renderer.get());
        instance.defaultTexture = createTexture(&instance, "default.jpg");
        instance.lightCount = 0;

        return instance;
    }

    void updateTransform(entt::registry &r, entt::entity e) {
        auto instance = r.ctx<Instance *>();
        if (auto model = r.try_get<impl::ModelRes>(e)) {
            glm::mat4 mat(1);
            auto &pos = r.get<Position>(e);
            mat = glm::translate(mat, pos);
            if (auto rotation = r.try_get<Rotation>(e)) {
                mat = glm::rotate(mat, glm::radians(90.f), *rotation / 90.f);
            }
            if (auto scale = r.try_get<Scale>(e)) {
                mat = glm::scale(mat, *scale);
            }
            updateUniformBuffer(instance->renderer.get(), model->uniformBuffer, mat);
        }
    }

    void initModel(entt::registry &r, entt::entity e) {
        if (r.try_get<impl::ModelRes>(e) ||
                !r.try_get<Drawable>(e) ||
                !r.try_get<Mesh>(e)) {
            return;
        }

        impl::ModelRes resource = {};
        auto instance = r.ctx<Instance *>();
        impl::ModelData data{glm::mat4(1)};
        resource.uniformBuffer = createUniformBuffer(instance->renderer.get(), data);

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = instance->layout;
        resourceHeapDesc.resourceViews.emplace_back(instance->globalShaderData);
        resourceHeapDesc.resourceViews.emplace_back(resource.uniformBuffer);

        if (auto texture = r.try_get<Texture>(e)) {
            resourceHeapDesc.resourceViews.emplace_back(instance->sampler);
            resourceHeapDesc.resourceViews.emplace_back(instance->resources.textures[texture->id]);
        } else {
            resourceHeapDesc.resourceViews.emplace_back(instance->sampler);
            resourceHeapDesc.resourceViews.emplace_back(instance->defaultTexture);
        }
        resource.heap = instance->renderer->CreateResourceHeap(resourceHeapDesc);

        r.emplace<impl::ModelRes>(e, resource);
        updateTransform(r, e);
    }

    void updateTexture(entt::registry &r, entt::entity e) {
        auto resource = r.try_get<impl::ModelRes>(e);
        if (!resource || !r.try_get<Drawable>(e) || !r.try_get<Mesh>(e)) {
            return;
        }

        auto instance = r.ctx<Instance *>();

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = instance->layout;
        resourceHeapDesc.resourceViews.emplace_back(instance->globalShaderData);
        resourceHeapDesc.resourceViews.emplace_back(resource->uniformBuffer);

        auto texture = r.get<Texture>(e);
        resourceHeapDesc.resourceViews.emplace_back(instance->sampler);
        resourceHeapDesc.resourceViews.emplace_back(instance->resources.textures[texture.id]);
        resource->heap = instance->renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void move(glm::vec3 &position, glm::vec3 direction, float speed) {
        auto time = static_cast<float>(events::pullTime().count());
        position += speed * direction * (time / 17.f);
    }

    glm::vec3 calcOrigin(glm::vec3 position, glm::vec3 rotation) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.z));
        direction.y = sin(glm::radians(rotation.z));
        direction.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.z));
        glm::vec3 cameraFront = glm::normalize(direction);
        return position + direction * 3.f;
    }

    void processKeyboard(Instance *instance, glm::vec3 &position, glm::vec3 rotation) {
        glm::vec3 origin = calcOrigin(position, rotation);
        glm::vec3 direction = glm::normalize(position - origin);

        glm::vec3 right = glm::normalize(glm::cross({0.0f, 1.0f, 0.0f}, direction));
        glm::vec3 up = glm::cross(direction, right);
        glm::vec3 front = glm::cross(up, right);

        float const speed = 0.05f;

        if (events::isDown(SDLK_f)) {
            instance->camera.relative = !instance->camera.relative;
            events::relativeMode(instance->camera.relative);
        }

        if (events::isPressed(SDLK_d)) {
            move(position, right, speed);
        }

        if (events::isPressed(SDLK_a)) {
            move(position, -right, speed);
        }

        if (events::isPressed(SDLK_w)) {
            move(position, front, speed);
        }

        if (events::isPressed(SDLK_s)) {
            move(position, -front, speed);
        }

        if (events::isPressed(SDLK_e)) {
            move(position, up, speed);
        }

        if (events::isPressed(SDLK_q)) {
            move(position, -up, speed);
        }
    }

    void processMouse(Instance *instance, glm::vec3 &rotation) {
        if (instance->camera.relative) {
            float const speed = 0.5f;
            auto offset = events::mouseOffset() * speed;
            rotation.x += offset.x;
            rotation.z -= offset.y;
        }
    }

    void processEvents(entt::registry &r, entt::entity camera) {
        auto instance = r.ctx<Instance *>();
        auto &position = r.get<Position>(camera);
        glm::vec3 *rotation = r.try_get<Rotation>(camera);
        if (!rotation) {
            rotation = &r.emplace<Rotation>(camera, glm::vec3(0, 0, 0));
        }

        glm::vec3 origin = calcOrigin(position, r.get<Rotation>(camera));

        mapUniformBuffer<impl::GlobalShaderData>(instance->renderer.get(),
                instance->globalShaderData, [=](impl::GlobalShaderData *data) {
                    data->view = glm::lookAt(position, origin, glm::vec3(0, 1, 0));
                    auto windowSize = instance->context->GetResolution();
                    data->projection = glm::perspective(glm::radians(45.0f),
                            static_cast<float>(windowSize.width) /
                                    static_cast<float>(windowSize.height), 0.1f, 100.0f);
                });

        processKeyboard(instance, position, *rotation);
        processMouse(instance, *rotation);
    }

    void updatePointLight(entt::registry &r, entt::entity e) {
        auto instance = r.ctx<Instance *>();

        auto position = r.try_get<Position>(e);
        auto light = r.try_get<PointLight>(e);

        if (!position || !light) {
            return;
        }

        size_t id = instance->lightCount;
        if (auto pId = r.try_get<impl::LightId>(e)) {
            id = pId->id;
        } else {
            if (id >= maxLightCount) {
                throw std::runtime_error("too many point lights");
            }
            r.emplace<impl::LightId>(e, id);
        }

        mapUniformBuffer<impl::GlobalShaderData>(instance->renderer.get(),
                instance->globalShaderData, [=, &r](impl::GlobalShaderData *data) {
                    data->pointLights[id].position = *position;
                    data->pointLights[id].diffuse = *position;
                    data->pointLights[id].constant = light->constant;
                    data->pointLights[id].linear = light->linear;
                    data->pointLights[id].quadratic = light->quadratic;
                    if (auto color = r.try_get<DiffuseColor>(e)) {
                        data->pointLights->diffuse = *color;
                    } else {
                        data->pointLights->diffuse = glm::vec3(1);
                    }
                });
    }

    void init(entt::registry &r, Instance *instance) {
        r.set<Instance *>(instance);

        r.on_construct<Mesh>().connect<&initModel>();
        r.on_construct<Drawable>().connect<&initModel>();
        r.on_construct<Position>().connect<&updateTransform>();
        r.on_update<Position>().connect<&updateTransform>();
        r.on_construct<Rotation>().connect<&updateTransform>();
        r.on_update<Rotation>().connect<&updateTransform>();
        r.on_construct<Scale>().connect<&updateTransform>();
        r.on_update<Scale>().connect<&updateTransform>();
        r.on_update<Texture>().connect<&updateTexture>();

        r.on_construct<PointLight>().connect<&updatePointLight>();
        r.on_construct<Position>().connect<&updatePointLight>();
        r.on_construct<DiffuseColor>().connect<&updatePointLight>();

        initGui(r);
    }

    std::pair<std::vector<Vertex>, std::vector<uint32_t>> loadMesh(
            tinyobj::attrib_t const &attrib, std::vector<tinyobj::shape_t> const &shapes) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (const auto &idx : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                vertex.texCoord.y = attrib.texcoords[2 * idx.texcoord_index + 1];

                vertices.push_back(vertex);

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);

            }
        }

        return {vertices, indices};
    }

    Mesh loadMesh(entt::registry &r, std::string const &path) {
        auto instance = r.ctx<Instance *>();

        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = false;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(instance->root + "/models/" + path, readerConfig)) {
            if (!reader.Error().empty()) {
                std::string err = "TinyObjReader: " + reader.Error();
                throw std::runtime_error(err);
            }
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto[vertices, indices] = loadMesh(reader.GetAttrib(), reader.GetShapes());

        auto vertexBuffer = createVertexBuffer(instance->renderer.get(), Vertex::format(),
                vertices);

        auto indexBuffer = createIndexBuffer(instance->renderer.get(), indices);

        impl::MeshRes res{vertexBuffer, indexBuffer, static_cast<uint32_t>(indices.size())};
        instance->resources.meshes.push_back(res);
        return Mesh{instance->resources.meshes.size() - 1};
    }

    Texture loadTexture(entt::registry &r, std::string const &path) {
        auto instance = r.ctx<Instance *>();
        instance->resources.textures.push_back(createTexture(instance, path));
        return {instance->resources.textures.size() - 1};
    }


    void setActiveCamera(entt::registry &r, entt::entity e, CameraMode) {
        auto instance = r.ctx<Instance *>();
        instance->camera.camera = e;
    }

    void renderLoop(entt::registry &r) {
        auto instance = r.ctx<Instance *>();
        LLGL::CommandQueue *cmdQueue = instance->renderer->GetCommandQueue();
        LLGL::CommandBuffer *cmdBuffer = instance->renderer->CreateCommandBuffer();

        while (events::pull(instance->window)) {
            cmdBuffer->Begin();
            cmdBuffer->BeginRenderPass(*instance->context);
            cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);
            cmdBuffer->SetViewport(instance->context->GetResolution());

            renderGui(r, cmdBuffer);

            cmdBuffer->SetPipelineState(*instance->pipeline);

            if (instance->camera.camera != entt::null) {
                processEvents(r, instance->camera.camera);
            }

            r.view<Drawable, Mesh, impl::ModelRes>().each(
                    [=](entt::entity, Drawable, Mesh meshId, impl::ModelRes res) {
                        auto &mesh = instance->resources.meshes[meshId.id];

                        cmdBuffer->SetResourceHeap(*res.heap);
                        cmdBuffer->SetVertexBuffer(*mesh.vertices);
                        cmdBuffer->SetIndexBuffer(*mesh.indices);
                        cmdBuffer->DrawIndexed(mesh.numIndices, 0);
                    });

            cmdBuffer->EndRenderPass();
            cmdBuffer->End();
            cmdQueue->Submit(*cmdBuffer);
            instance->context->Present();
        }
    }

}