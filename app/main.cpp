#include <iostream>
#include "rendering.hpp"

using namespace rise;

entt::entity addModel(entt::registry &r, std::string const& name, Mesh mesh, Texture texture,
        glm::vec3 pos, glm::vec3 rotation = glm::vec3(0, 0, 0), glm::vec3 scale = glm::vec3(1)) {
    entt::entity cube = r.create();
    r.emplace<Mesh>(cube, mesh);
    r.emplace<Texture>(cube, texture);
    r.emplace<Position>(cube, pos);
    r.emplace<Scale>(cube, scale);
    r.emplace<Name>(cube, name);
    r.emplace<Rotation>(cube, rotation);
    r.emplace<Drawable>(cube, Shading::Diffuse);
    return cube;
}

entt::entity addLight(entt::registry& r, std::string const& name, glm::vec3 position, Mesh mesh,
        Texture texture) {
    auto light = addModel(r, name, mesh, texture, position,
            glm::vec3(270, 0, 0), glm::vec3(0.2, 0.2, 0.2));
    r.emplace<PointLight>(light, 25.0f, 10.0f);
    r.emplace<DiffuseColor>(light, glm::vec3(1.0, 1.0f, 1.0f));
    return light;
}

int main() {
    entt::registry registry;
    //Instance instance = makeInstance("/home/gaben/projects/rise", 1600, 1000);
    Instance instance = makeInstance("C:/Users/smirn/source/repos/rise2", 1200, 800);
    init(registry, &instance);

    Mesh cube = loadMesh(registry, "cube.obj");
    //Mesh lamp = loadMesh(registry, "Nicolas_Aubagnac_Durer_S_mat(1).obj");
    Texture texture = loadTexture(registry, "default.jpg");

    addModel(registry, "cube1", cube, texture, glm::vec3(0, 1, 0));
    addModel(registry, "cube2", cube, texture, glm::vec3(1, 0, 0));
    addModel(registry, "cube3", cube, texture, glm::vec3(-1, 0, 0));
    addModel(registry, "cube4", cube, texture, glm::vec3(-2, 1, 0));

    auto camera = registry.create();
    registry.emplace<Position>(camera, glm::vec3(-6, 2, 1));

    addLight(registry, "lamp1", glm::vec3{0.f, 3.f, 2.f}, cube, texture);
    addLight(registry, "lamp2", glm::vec3{0.f, 3.f, -2.f}, cube, texture);

    setActiveCamera(registry, camera, CameraMode::FullControl);

    renderLoop(registry);
}
