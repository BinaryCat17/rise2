#pragma once

#include "mesh.hpp"
#include "resources.hpp"
#include <entt/entt.hpp>

namespace rise {
	unsigned const MeshDataBinding = 0;
	unsigned const CameraBinding = 0;

	struct SceneResources {
		LLGL::PipelineLayout* layout;
		LLGL::Buffer* camera;
		std::vector<LLGL::ResourceHeap*> heaps;
		std::vector<LLGL::Buffer*> meshData;
		std::vector<Mesh> meshes;

		struct MeshDataId {
			size_t index;
			unsigned binding;
		};

		struct MeshId {
			size_t index;
		};
	};

	struct Position : glm::vec3 {};

	struct Rotation : glm::vec3 {};

	struct Scale : glm::vec3 {};

	void makeModel(entt::registry& r, entt::entity entity, size_t meshId, Position position);

	void initScene(entt::registry& r);

	void renderScene(LLGL::CommandBuffer* cmd, entt::registry& r);
}