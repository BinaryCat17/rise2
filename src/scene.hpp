#pragma once

#include "mesh.hpp"
#include "rendering.hpp"
#include <entt/entt.hpp>

namespace rise::scene {
	struct ModelData {
		Mesh mesh;
		LLGL::Buffer* uniformBuffer;
		LLGL::ResourceHeap* heap;
	};

	struct Position : glm::vec3 {};

	struct Rotation : glm::vec3 {};

	struct Scale : glm::vec3 {};

	void makeModel(entt::registry& r, entt::entity entity, Mesh mesh, Position position);

	void renderScene(LLGL::CommandBuffer* cmd, entt::registry& r);
}