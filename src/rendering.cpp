#include "rendering.hpp"

#include "resources.hpp"
#include "pipeline.hpp"
#include "input.hpp"
#include <LLGL/LLGL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "platform.hpp"
#include "renderGui.hpp"
#include "ecs.hpp"

namespace rise {
	struct ModelRes {
		LLGL::Buffer* objectBuffer;
		LLGL::Buffer* materialBuffer;
		LLGL::Texture* texture;
		LLGL::ResourceHeap* heap;
	};

	struct Instance {
		std::vector<std::function<void(flecs::world& ecs)>> imguiCallbacks;
		std::unique_ptr<LLGL::RenderSystem> renderer = nullptr;
		SDL_Window* window = nullptr;
		LLGL::RenderContext* context = nullptr;
		PipelineData pipeline;
		LLGL::Buffer* globalData = nullptr;
		TextureRes defaultTexture;
		LLGL::Sampler* sampler = nullptr;
		LLGL::CommandBuffer* cmdBuf = nullptr;
		LLGL::CommandQueue* cmdQueue = nullptr;
	};

	struct RenderState {
		std::string root;
		Instance instance;
		flecs::query<Position const*, Color const*, Distance const*,
			Intensity const*, PointLight> lightQuery;
	};

	LLGL::RenderContext* makeContext(LLGL::RenderSystem* renderer, SDL_Window* window,
		unsigned width, unsigned height) {
		LLGL::RenderContextDescriptor contextDesc;
		contextDesc.videoMode.resolution = { width, height };
		contextDesc.videoMode.fullscreen = false;
		contextDesc.vsync.enabled = true;
		contextDesc.samples = 8;
		LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc,
			std::make_shared<Surface>(window));

		const auto& info = renderer->GetRendererInfo();

		std::cout << "Renderer:         " << info.rendererName << std::endl;
		std::cout << "Device:           " << info.deviceName << std::endl;
		std::cout << "Vendor:           " << info.vendorName << std::endl;
		std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

		return context;
	}

	class Debugger : public LLGL::RenderingDebugger {
	public:
	protected:
		void OnError(LLGL::ErrorType type, Message& message) override {
			//std::cerr << "error: " << message.GetText() << std::endl;
		}

		void OnWarning(LLGL::WarningType type, Message& message) override {
			//std::cerr << "warning: " << message.GetText() << std::endl;
		}
	};

	std::unique_ptr<LLGL::RenderSystem> createRenderer() {
		static Debugger debugger;
		LLGL::Log::SetReportCallbackStd(std::cerr);
		return LLGL::RenderSystem::Load("Vulkan", nullptr, &debugger);
	}

	Instance makeInstance(std::string const& root, SDL_Window* window) {
		Instance instance;

		instance.renderer = createRenderer();
		auto renderer = instance.renderer.get();

		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		instance.context = makeContext(renderer, window, width, height);
		instance.window = window;
		instance.pipeline = scenePipeline::make(renderer, root);
		instance.globalData = createUniformBuffer(renderer, scenePipeline::Global{});
		instance.sampler = makeSampler(renderer);
		instance.defaultTexture = loadTextureFromDisk(renderer, root + "/textures/default.jpg");
		instance.cmdQueue = instance.renderer->GetCommandQueue();
		instance.cmdBuf = instance.renderer->CreateCommandBuffer();

		return instance;
	}

	RenderState makeState(flecs::world& ecs, std::string const& root) {
		RenderState state;
		state.root = root;
		state.instance = makeInstance(root, createWindow());
		state.lightQuery = ecs.query<Position const*, Color const*,
			Distance const*, Intensity const*, PointLight>();
	}

	void updateCamera(flecs::world& ecs, scenePipeline::Global* data) {
		auto& instance = checkGet<RenderState>(ecs).instance;
		auto position = getOrDefault<CameraPosition>(ecs);
		auto rotation = getOrDefault<CameraRotation>(ecs);

		glm::vec3 origin = calcOrigin(position->val, rotation->val);

		data->view = glm::lookAt(position->val, origin, glm::vec3(0, 1, 0));
		auto windowSize = instance.context->GetResolution();
		data->projection = glm::perspective(glm::radians(45.0f),
			static_cast<float>(windowSize.width) /
			static_cast<float>(windowSize.height), 0.1f, 100.0f);
	}

	void updatePointLight(flecs::world& ecs, scenePipeline::Global* data) {
		auto& state = checkGet<RenderState>(ecs);
		state.lightQuery.iter([](flecs::iter it, Position const*,
			Color const*, Distance const*, Intensity const*) {

			});

			for (size_t i = 0; i != scenePipeline::maxLightCount; ++i) {
				auto& light = data->pointLights[i];
				if (i < entities.size()) {
					auto e = entities[i];
					if (auto position = deepTryGet<Position>(r, e)) {
						light.position = position->val;
					}
					else {
						light.position = glm::vec3(0.f, 0.f, 0.f);
					}
					if (auto color = deepTryGet<DiffuseColor>(r, e)) {
						light.diffuse = color->val;
					}
					else {
						light.diffuse = glm::vec3(1.f, 1.f, 1.f);
					}
					if (auto distance = deepTryGet<Distance>(r, e)) {
						light.distance = distance->val;
					}
					else {
						light.distance = 10.f;
					}
					if (auto intensity = deepTryGet<Intensity>(r, e)) {
						light.intensity = intensity->val;
					}
					else {
						light.intensity = 1.f;
					}
				}
				else {
					light.diffuse = glm::vec3(0, 0, 0);
				}
			}
	}

	void updateGlobal(entt::registry& r) {
		auto& state = r.ctx<RenderState>();
		auto& instance = state.instance;

		mapUniformBuffer<scenePipeline::Global>(instance.renderer.get(),
			instance.globalData, [&r, &instance, &state](scenePipeline::Global* pData) {
				updateCamera(r, state.camera, instance, pData);
				updatePointLight(r, instance, pData);
			});
	}

	LLGL::ResourceHeap* makeResourceHeap(Instance& instance, ModelRes& model) {
		LLGL::ResourceHeapDescriptor resourceHeapDesc;
		resourceHeapDesc.pipelineLayout = instance.pipeline.layout;
		resourceHeapDesc.resourceViews.emplace_back(instance.globalData);
		resourceHeapDesc.resourceViews.emplace_back(model.materialBuffer);
		resourceHeapDesc.resourceViews.emplace_back(model.objectBuffer);
		resourceHeapDesc.resourceViews.emplace_back(instance.sampler);
		resourceHeapDesc.resourceViews.emplace_back(model.texture);
		return instance.renderer->CreateResourceHeap(resourceHeapDesc);
	}

	void updateObjectResources(entt::registry& r, entt::entity e, Instance& instance,
		ModelRes& model) {
		auto texture = deepTryGet<TextureRes>(r, e);
		if (!texture) {
			texture = &instance.defaultTexture;
		}

		if (model.texture != texture->texture) {
			model.texture = texture->texture;
			model.heap = makeResourceHeap(instance, model);
		}
	}

	void updateTransform(entt::registry& r, entt::entity e, Instance& instance, ModelRes& model) {
		glm::mat4 mat(1);
		if (auto position = deepTryGet<Position>(r, e)) {
			mat = glm::translate(mat, position->val);
		}

		if (auto rotation = deepTryGet<Rotation>(r, e)) {
			if (rotation->val != glm::vec3(0)) {
				float angle = std::max({ rotation->val.x, rotation->val.y, rotation->val.z });
				mat = glm::rotate(mat, glm::radians(angle), glm::normalize(rotation->val));
			}
		}

		if (auto scale = deepTryGet<Scale>(r, e)) {
			mat = glm::scale(mat, scale->val);
		}

		updateUniformBuffer(instance.renderer.get(), model.objectBuffer,
			scenePipeline::PerObject{ mat });
	}

	void updateMaterial(entt::registry& r, entt::entity e, Instance& instance, ModelRes& model) {
		mapUniformBuffer<scenePipeline::PerMaterial>(instance.renderer.get(),
			model.materialBuffer,
			[&r, e](scenePipeline::PerMaterial* pData) {
				if (auto color = deepTryGet<DiffuseColor>(r, e)) {
					pData->diffuseColor = color->val;
				}
				else {
					pData->diffuseColor = glm::vec3(0.5, 0.5, 0.5);
				}
			});
	}

	void updateModel(entt::registry& r, entt::entity e, Instance& instance, ModelRes& model) {
		updateObjectResources(r, e, instance, model);
		updateTransform(r, e, instance, model);
		updateMaterial(r, e, instance, model);
	}

	ModelRes* initModel(entt::registry& r, entt::entity e, Instance& instance) {
		auto& res = r.emplace<ModelRes>(e);
		res.texture = instance.defaultTexture.texture;
		scenePipeline::PerObject objectData{ glm::mat4(1.f) };
		res.objectBuffer = createUniformBuffer(instance.renderer.get(), objectData);
		scenePipeline::PerMaterial materialData{ glm::vec3(1, 1, 1) };
		res.materialBuffer = createUniformBuffer(instance.renderer.get(), materialData);
		res.heap = makeResourceHeap(instance, res);
		return &res;
	}

	void RenderSystem::init(entt::registry& r) {
		auto parameters = r.ctx_or_set<Parameters>();
		auto& state = r.set<RenderState>();
		state.root = parameters.root;

		state.instance = makeInstance(parameters.root, InputSystem::getWindow(r));
		initGui(r, state.instance.renderer.get(), state.instance.window, state.root);
	}

	bool RenderSystem::update(entt::registry& r) {
		auto& state = r.ctx<RenderState>();
		auto& instance = state.instance;
		auto cmdBuffer = instance.cmdBuf;

		cmdBuffer->Begin();
		cmdBuffer->BeginRenderPass(*instance.context);
		cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);
		auto resolution = instance.context->GetResolution();

		cmdBuffer->SetViewport(resolution);
		cmdBuffer->SetPipelineState(*instance.pipeline.pipeline);

		updateGlobal(r);
		r.view<Drawable>().each([&r, &instance, cmdBuffer](entt::entity e) {
			auto model = r.try_get<ModelRes>(e);
			if (!model) {
				model = initModel(r, e, instance);
			}
			auto* mesh = deepTryGet<MeshRes>(r, e);
			if (mesh) {
				updateModel(r, e, instance, *model);
				cmdBuffer->SetResourceHeap(*model->heap);
				cmdBuffer->SetVertexBuffer(*mesh->vertices);
				cmdBuffer->SetIndexBuffer(*mesh->indices);
				cmdBuffer->DrawIndexed(mesh->numIndices, 0);
			}
			});

		renderGui(r, state.instance.renderer.get(), state.instance.window, cmdBuffer);

		cmdBuffer->EndRenderPass();
		cmdBuffer->End();
		instance.cmdQueue->Submit(*cmdBuffer);
		instance.context->Present();

		return true;
	}

	void RenderSystem::destroy(entt::registry&) {

	}

	void RenderSystem::setActiveCamera(entt::registry& r, entt::entity camera) {
		auto& state = r.ctx<RenderState>();
		state.camera = camera;
		auto position = deepTryGet<Position>(r, camera);
		auto rotation = deepTryGet<Rotation>(r, camera);
		if (!position) {
			r.emplace<Position>(camera, glm::vec3(0, 0, 0));
		}
		if (!rotation) {
			r.emplace<Rotation>(camera, glm::vec3(0, 0, 0));
		}
	}

	entt::entity RenderSystem::getActiveCamera(entt::registry& r) {
		auto& state = r.ctx<RenderState>();
		return state.camera;
	}

	void RenderSystem::loadTexture(entt::registry& r, entt::entity e, const std::string& path) {
		auto& state = r.ctx<RenderState>();
		r.emplace<TextureRes>(e, loadTextureFromDisk(state.instance.renderer.get(),
			state.root + "/textures/" + path));
		r.emplace<Texture>(e);
		r.emplace<Path>(e, path);
	}

	void RenderSystem::loadMesh(entt::registry& r, entt::entity e, const std::string& path) {
		auto& state = r.ctx<RenderState>();
		r.emplace<MeshRes>(e, loadMeshFromDisk(state.instance.renderer.get(),
			state.root + "/models/" + path, state.instance.pipeline.format));
		r.emplace<Mesh>(e);
		r.emplace<Path>(e, path);
	}

	entt::delegate<void(entt::registry& r)>& RenderSystem::imguiCallback(entt::registry& r) {
		return guiCallbacks(r);
	}

	RenderModule::RenderModule(flecs::world& ecs) {
		ecs.module<RenderModule>();
		ecs.set<CameraPosition>({ glm::vec3{} });
		ecs.set<CameraRotation>({ glm::vec3{} });
	}
}