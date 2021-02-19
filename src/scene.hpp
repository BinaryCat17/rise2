#pragma once

#include "mesh.hpp"
#include "resources.hpp"
#include "rendering.hpp"
#include <entt/entt.hpp>

namespace rise {
	unsigned const MeshDataBinding = 0;
	unsigned const CameraBinding = 0;

    struct MeshDataId {
        size_t index;
    };

    struct MeshId {
        size_t index;
    };

	struct SceneResources {
	    std::unique_ptr<LLGL::RenderSystem> renderer;
	    Context context;
	    Pipeline pipeline;
		LLGL::Buffer* camera;
		std::vector<LLGL::ResourceHeap*> heaps;
		std::vector<LLGL::Buffer*> meshData;
		std::vector<Mesh> meshes;
	};

	struct Position : glm::vec3 {};

	struct Rotation : glm::vec3 {};

	struct Scale : glm::vec3 {};

	void makeModel(entt::registry& r, entt::entity entity, size_t meshId, Position position);

	void initScene(entt::registry& r);

	void renderScene(LLGL::CommandBuffer* cmd, entt::registry& r);
}