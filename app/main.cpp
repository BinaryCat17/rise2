#include <rise/rendering/module.hpp>

using namespace rise;

int main() {
    flecs::world ecs;
    RenderModule::setWorkDirectory(ecs, "/home/gaben/projects/rise");
    ecs.import<RenderModule>();

    auto cube = ecs.entity("Cube").set<WorldPosition>({glm::vec3(0, 0, 0)});

    auto camera = ecs.entity("Camera")
            .set<WorldPosition>({glm::vec3(0, 1, 0)})
            .set<DiffuseColor>({glm::vec3(1, 1, 1)})
            .set<Intensity>({1.f})
            .set<Viewport>({{0, 0}, {800, 600}});

    RenderModule::renderTo(ecs, cube, camera);

    while (ecs.progress()) {

    }
}
