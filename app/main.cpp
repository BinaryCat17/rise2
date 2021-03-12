#include <rise/systems/rendering/module.hpp>
#include <rise/components/rendering/module.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/util/flecs_os.hpp>

using namespace rise::systems;
using namespace rise::components::rendering;

flecs::world initWorld() {
    stdcpp_set_os_api();

    flecs::world ecs;
    ecs.import<flecs::components::meta>();
    ecs.import<Rendering>();

    ecs.import<flecs::dash>();
    ecs.import<flecs::systems::civetweb>();
    ecs.entity().set<flecs::dash::Server>({9090});

    return ecs;
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.entity().set<Extent2D>({1600, 1200});
    auto application = ecs.entity("Minecraft 2").add_instanceof(windowSize);
    Rendering::regApplication(application);

    auto camera = ecs.entity("Viewport").
            add_instanceof(windowSize).
            set<Position3D>({1, 1, 1}).
            set<Distance>({5.f});
    Rendering::regViewport(application, camera);
    Rendering::regPointLight(application, camera);

    auto mesh = ecs.entity("CubeMesh").set<Path>({"cube.obj"});
    Rendering::regMesh(application, mesh);

    auto texture = ecs.entity("CubeTexture").set<Path>({"paper.jpg"});
    Rendering::regTexture(application, texture);

    auto cube = ecs.entity("Cube").
            add_instanceof(mesh).
            add_instanceof(texture).
            set<Position3D>({0, 0, 0});
    Rendering::regModel(application, texture);

    cube.add_childof(camera);

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
