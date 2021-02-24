#include <iostream>
#include "rendering.hpp"

using namespace rise;

entt::entity addModel(entt::registry &r, Mesh mesh, Texture texture, glm::vec3 pos,
        glm::vec3 scale = glm::vec3(1)) {
    entt::entity cube = r.create();
    r.emplace<Mesh>(cube, mesh);
    r.emplace<Texture>(cube, texture);
    r.emplace<Position>(cube, pos);
    r.emplace<Scale>(cube, scale);
    r.emplace<Drawable>(cube, Shading::Diffuse);
    return cube;
}

int main() {
    entt::registry registry;
    //Instance instance = makeInstance("/home/gaben/projects/rise", 1600, 1000);
    Instance instance = makeInstance("C:/Users/smirn/source/repos/rise2", 800, 600);
    init(registry, &instance);

    Mesh cube = loadMesh(registry, "cube.obj");
    Texture texture = loadTexture(registry, "default.jpg");

    addModel(registry, cube, texture, glm::vec3(0, 1, 0));
    addModel(registry, cube, texture, glm::vec3(1, 0, 0));
    addModel(registry, cube, texture, glm::vec3(-1, 0, 0));
    addModel(registry, cube, texture, glm::vec3(-2, 1, 0));

    auto camera = registry.create();
    registry.emplace<Position>(camera, glm::vec3(3, 3, 0));

    auto light = registry.create();
    registry.emplace<Position>(light, glm::vec3(3, 3, 4));
    registry.emplace<PointLight>(light, 1.f, 0.045f, 0.0075f);

    setActiveCamera(registry, camera, CameraMode::FullControl);

    renderLoop(registry);
}
