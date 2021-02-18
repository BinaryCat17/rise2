#include "scene.hpp"
#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
	void makeModel(entt::registry& r, entt::entity entity, size_t meshId, Position position) {
		auto& resources = r.ctx<SceneResources>();
		if (meshId >= resources.meshes.size()) {
			throw std::runtime_error("mesh doesn't exist");
		}

		r.emplace<Position>(entity, position);
		r.emplace<SceneResources::MeshId>(entity, meshId);
		glm::mat4 transformMatrix = glm::translate(glm::mat4(1), position);

		auto renderer = r.ctx<LLGL::RenderSystem*>();
		auto buffer = createUniformBuffer(renderer, transformMatrix);
		resources.meshData.push_back(buffer);

		LLGL::ResourceHeapDescriptor resourceHeapDesc;
		resourceHeapDesc.pipelineLayout = resources.layout;
		resourceHeapDesc.resourceViews.emplace_back(resources.camera);
		resourceHeapDesc.resourceViews.emplace_back(buffer);
		resources.heaps.emplace_back(renderer->CreateResourceHeap(resourceHeapDesc));

		r.emplace<SceneResources::MeshDataId>(entity,
			resources.meshData.size() - 1,
			MeshDataBinding);
	}

	void reCalc(entt::registry& r, entt::entity e) {
		glm::mat4 mat(1);
		auto& pos = r.get<Position>(e);
		mat = glm::translate(mat, pos);
		if (auto rotation = r.try_get<Rotation>(e)) {
			mat = glm::rotate(mat, glm::radians(90.f), *rotation);
		}
		if (auto scale = r.try_get<Scale>(e)) {
			mat = glm::scale(mat, *scale);
		}

		auto& resources = r.ctx<SceneResources>();
		auto buf = resources.meshData[r.get<SceneResources::MeshDataId>(e).index];
		auto renderer = r.ctx<LLGL::RenderSystem*>();
		mapUniformBuffer<glm::mat4>(renderer, buf, [&mat](glm::mat4* m) {
			*m = mat;
			});
	}

	void initScene(entt::registry& r) {
		auto& resources = r.set<SceneResources>();
		auto renderer = r.ctx<LLGL::RenderSystem*>();

		LLGL::PipelineLayoutDescriptor layoutDesc;
		layoutDesc.bindings = { LLGL::BindingDescriptor{
						LLGL::ResourceType::Buffer,
						LLGL::BindFlags::ConstantBuffer,
						LLGL::StageFlags::VertexStage,
						CameraBinding,
					}, { LLGL::BindingDescriptor {
						LLGL::ResourceType::Buffer,
						LLGL::BindFlags::ConstantBuffer,
						LLGL::StageFlags::VertexStage,
						MeshDataBinding,
					}}
		};

		resources.layout = renderer->CreatePipelineLayout(layoutDesc);
		resources.camera = createUniformBuffer(renderer, CameraData{});

		r.on_construct<Position>().connect<&reCalc>();
		r.on_update<Position>().connect<&reCalc>();

		r.on_construct<Rotation>().connect<&reCalc>();
		r.on_update<Rotation>().connect<&reCalc>();

		r.on_construct<Scale>().connect<&reCalc>();
		r.on_update<Scale>().connect<&reCalc>();
	}

	void renderScene(LLGL::CommandBuffer* cmd, entt::registry& r) {
		auto& resources = r.ctx<SceneResources>();
		r.view<SceneResources::MeshDataId, SceneResources::MeshId>().
			each([cmd, &resources](auto entity, auto dataId, auto meshId) {
			cmd->SetResourceHeap(resources.heaps[dataId]);
			cmd->SetVertexBuffer(resources.meshes[meshId].vertices);
			cmd->SetIndexBuffer(resources.meshes[meshId].indices);
			cmd->DrawIndexed(resources.meshes[meshId].numIndices);
				});
	}
}