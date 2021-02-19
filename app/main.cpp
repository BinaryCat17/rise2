#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering.hpp"
#include <entt/entt.hpp>

int main(int argc, char* argv[]) {
	using namespace rise;

	try {
		entt::registry registry;
		Instance instance = makeInstance("/home/gaben/projects/rise", 800, 600);

		init(registry, &instance);

		Mesh mesh = loadMesh(registry, "/meshes/cube.obj");

		entt::entity cube = registry.create();
		registry.emplace<Mesh>(cube, &mesh);
		registry.emplace<Position>(mesh, glm::vec3(0, 0, 0));

		renderLoop(registry);
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << std::endl;
	}
}
