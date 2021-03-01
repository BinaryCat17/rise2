#include <rendering.hpp>
#include <input.hpp>
#include <system.hpp>

using namespace rise;

struct GameSystem {
    static void init(entt::registry &r) {
        auto cube = r.create();
        RenderSystem::loadMesh(r, cube, "cube.obj");

        auto texture = r.create();
        RenderSystem::loadTexture(r, texture, "default.jpg");

        auto model = r.create();
        r.emplace<SubEntities>(model, SubEntities{ { cube, texture } });
        r.emplace<DiffuseColor>(model, glm::vec3(0.6, 0.6, 0.f));
        r.emplace<Drawable>(model);

        auto camera = r.create();
        r.emplace<Position>(camera, glm::vec3(0, 1, 0));
        r.emplace<PointLight>(camera);

        RenderSystem::setActiveCamera(r, camera);
    }

    static bool update(entt::registry &r) {
        return true;
    }

    static void destroy(entt::registry &r) {}
};

int main() {
    rise::Parameters params = {};
    params.root = "/home/gaben/projects/rise";

    rise::start<InputSystem, RenderSystem, GameSystem>(params);
}
