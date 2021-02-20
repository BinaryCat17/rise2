#include <iostream>
#include "rendering.hpp"

using namespace rise;

void addModel(entt::registry &r, Mesh mesh, glm::vec3 pos) {
    entt::entity cube = r.create();
    r.emplace<Mesh>(cube, mesh);
    r.emplace<Position>(cube, pos);
    r.emplace<Drawable>(cube, Shading::Diffuse);
}

int main() {
    try {
        entt::registry registry;
        Instance instance = makeInstance("/home/gaben/projects/rise", 800, 600);
        init(registry, &instance);

        Mesh cube = loadMesh(registry, "cube.obj");

        addModel(registry, cube, glm::vec3(0, 1, 0));
        addModel(registry, cube, glm::vec3(1, 0, 0));
        addModel(registry, cube, glm::vec3(-1, 0, 0));
        addModel(registry, cube, glm::vec3(-2, 1, 0));

        auto camera = registry.create();
        registry.emplace<Position>(camera, glm::vec3(3, 3, 4));
        registry.emplace<Rotation>(camera, glm::vec3(0, 0, 0));

        setActiveCamera(registry, camera, CameraMode::FullControl);

        renderLoop(registry);
    }
    catch (std::exception const &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
