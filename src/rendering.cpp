#include "rendering.hpp"
#include <iostream>
#include "pipeline.hpp"
#include "resources.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include <chrono>

namespace rise {
    LLGL::RenderContext *
    makeContext(LLGL::RenderSystem *renderer, unsigned width, unsigned height) {
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = {width, height};
        contextDesc.videoMode.fullscreen = false;
        contextDesc.vsync.enabled = true;
        contextDesc.samples = 8;
        LLGL::RenderContext *context = renderer->CreateRenderContext(contextDesc);

        const auto &info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        return context;
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
        Instance instance;

        instance.renderer = createRenderer();
        instance.context = makeContext(instance.renderer.get(), width, height);

        auto &window = LLGL::CastTo<LLGL::Window>(instance.context->GetSurface());
        window.SetTitle(L"Rise!");
        window.Show();

        instance.window = &window;

        instance.layout = makeLayout(instance.renderer.get());
        instance.program = makeProgram(instance.renderer.get(), root + "/shaders");
        instance.pipeline = makePipeline(instance.renderer.get(), instance.layout,
                instance.program);

        impl::GlobalShaderData data{glm::mat4(1), glm::mat4(1)};
        instance.globalShaderData = createUniformBuffer(instance.renderer.get(), data);

        instance.root = root;

        return instance;
    }

    void updateTransform(entt::registry &r, entt::entity e) {
        auto instance = r.ctx<Instance *>();
        auto &pos = r.get<Position>(e);
        if (auto model = r.try_get<impl::ModelRes>(e)) {
            glm::mat4 mat(1);
            mat = glm::translate(mat, pos);
            updateUniformBuffer(instance->renderer.get(), model->uniformBuffer, mat);
        }
    }

    void updateModel(entt::registry &r, entt::entity e) {
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
        resource.heap = instance->renderer->CreateResourceHeap(resourceHeapDesc);

        r.emplace<impl::ModelRes>(e, resource);
        updateTransform(r, e);
    }


    struct WindowListener : public LLGL::Window::EventListener {
    public:
        void OnKeyDown(LLGL::Window &sender, LLGL::Key keyCode) override {
            if (keyCode == LLGL::Key::W) {
                isMoveFront = true;
            }
            if (keyCode == LLGL::Key::S) {
                isMoveBack = true;
            }
            if (keyCode == LLGL::Key::A) {
                isMoveLeft = true;
            }
            if (keyCode == LLGL::Key::D) {
                isMoveRight = true;
            }
            if (keyCode == LLGL::Key::Q) {
                isMoveUp = true;
            }
            if (keyCode == LLGL::Key::E) {
                isMoveDown = true;
            }
        }

        void OnKeyUp(LLGL::Window &sender, LLGL::Key keyCode) override {
            if (keyCode == LLGL::Key::W) {
                isMoveFront = false;
            }
            if (keyCode == LLGL::Key::S) {
                isMoveBack = false;
            }
            if (keyCode == LLGL::Key::A) {
                isMoveLeft = false;
            }
            if (keyCode == LLGL::Key::D) {
                isMoveRight = false;
            }
            if (keyCode == LLGL::Key::Q) {
                isMoveUp = false;
            }
            if (keyCode == LLGL::Key::E) {
                isMoveDown = false;
            }
        }

        void OnLocalMotion(LLGL::Window &sender, const LLGL::Offset2D &position) override {
            float xOffset = static_cast<float>(position.x) - lastMouse.x;
            float yOffset = static_cast<float>(position.y) - lastMouse.y;
            lastMouse = { position.x, position.y };

            xOffset *= sensitivity;
            yOffset *= sensitivity;

            yaw   += xOffset;
            pitch += yOffset;

            if(pitch > 89.0f)
                pitch = 89.0f;
            if(pitch < -89.0f)
                pitch = -89.0f;

            glm::vec3 direction;
            direction.x = static_cast<float>(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
            direction.y = static_cast<float>(sin(glm::radians(pitch)));
            direction.z = static_cast<float>(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));

            auto &rotation = registry->get<Rotation>(camera);
            static_cast<glm::vec3&>(rotation) = glm::normalize(direction) / 90.f;
        }

        void OnProcessEvents(LLGL::Window &sender) override {
            if (registry == nullptr) {
                return;
            }

            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            auto elapsed = static_cast<float>(duration_cast<milliseconds>(now - lastTime).count());
            lastTime = now;

            auto instance = registry->ctx<Instance *>();
            auto &position = registry->get<Position>(camera);
            auto &rotation = registry->get<Rotation>(camera);

            glm::vec3 origin;
            if(rotation.x == 0 && rotation.y == 0 && rotation.z == 0) {
                 origin = position + glm::vec3(-1, 0, 0);
            } else {
                origin = position + (rotation / 90.f);
            }
            glm::vec3 direction = glm::normalize(position - origin);

            glm::vec3 right = glm::normalize(glm::cross({0.0f, 1.0f, 0.0f}, direction));
            glm::vec3 up = glm::cross(direction, right);
            glm::vec3 front = glm::cross(up, right);

            mapUniformBuffer<impl::GlobalShaderData>(instance->renderer.get(),
                    instance->globalShaderData, [=](impl::GlobalShaderData *data) {
                        data->view = glm::lookAt(position, origin, up);
                        auto windowSize = instance->context->GetResolution();
                        data->projection = glm::perspective(glm::radians(45.0f),
                                static_cast<float>(windowSize.width) /
                                        static_cast<float>(windowSize.height), 0.1f, 100.0f);
                    });

            if (isMoveRight) {
                position += speed * right * (elapsed / 17.f);
            }

            if (isMoveLeft) {
                position -= speed * right * (elapsed / 17.f);
            }

            if (isMoveFront) {
                position += speed * front * (elapsed / 17.f);
            }

            if (isMoveBack) {
                position -= speed * front * (elapsed / 17.f);
            }

            if (isMoveUp) {
                position += speed * up * (elapsed / 17.f);
            }

            if (isMoveDown) {
                position -= speed * up * (elapsed / 17.f);
            }
        }

        entt::registry *registry = nullptr;
        entt::entity camera{entt::null};
        std::chrono::high_resolution_clock::time_point lastTime;
        bool isMoveFront = false;
        bool isMoveBack = false;
        bool isMoveRight = false;
        bool isMoveLeft = false;
        bool isMoveUp = false;
        bool isMoveDown = false;
        glm::vec2 lastMouse;
        float yaw = -90.f;
        float pitch = 0.f;
        float speed = 0.05f;
        float sensitivity = 0.1f;
    };

    void init(entt::registry &r, Instance *instance) {
        r.set<Instance *>(instance);
        auto& listener = r.set<std::shared_ptr<WindowListener>>(std::make_shared<WindowListener>());
        instance->window->AddEventListener(listener);

        r.on_construct<Mesh>().connect<&updateModel>();
        r.on_construct<Drawable>().connect<&updateModel>();
        r.on_construct<Position>().connect<&updateTransform>();
        r.on_update<Position>().connect<&updateTransform>();
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

        LLGL::VertexFormat vertexFormat = Vertex::format();
        LLGL::BufferDescriptor VBufferDesc;
        VBufferDesc.size = sizeof(Vertex) * vertices.size();
        VBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        VBufferDesc.vertexAttribs = vertexFormat.attributes;

        auto vertexBuffer = instance->renderer->CreateBuffer(VBufferDesc, vertices.data());

        LLGL::BufferDescriptor IBufferDesc;
        IBufferDesc.size = sizeof(uint32_t) * indices.size();
        IBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        IBufferDesc.format = LLGL::Format::R32UInt;

        auto indexBuffer = instance->renderer->CreateBuffer(IBufferDesc, indices.data());

        impl::MeshRes res{vertexBuffer, indexBuffer, static_cast<uint32_t>(indices.size())};
        instance->resources.meshes.push_back(res);
        return Mesh{instance->resources.meshes.size() - 1};
    }

    void setActiveCamera(entt::registry &r, entt::entity e, CameraMode mode) {
        auto& listener = r.ctx<std::shared_ptr<WindowListener>>();
        listener->registry = &r;
        listener->camera = e;
    }

    void renderLoop(entt::registry &r) {
        auto instance = r.ctx<Instance *>();
        LLGL::CommandQueue *cmdQueue = instance->renderer->GetCommandQueue();
        LLGL::CommandBuffer *cmdBuffer = instance->renderer->CreateCommandBuffer();

        while (instance->window->ProcessEvents()) {
            cmdBuffer->Begin();
            cmdBuffer->BeginRenderPass(*instance->context);
            cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);
            cmdBuffer->SetPipelineState(*instance->pipeline);
            cmdBuffer->SetViewport(instance->context->GetResolution());

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