#include <rise/systems/rendering/module.hpp>
#include <rise/systems/input/module.hpp>
#include <rise/components/rendering/imgui.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/util/flecs_os.hpp>
#include <rise/editor/gui.hpp>

using namespace rise;
using namespace rise::systems;
using namespace rise::components::rendering;
using namespace rise::editor;

flecs::world initWorld() {
    stdcpp_set_os_api();

    flecs::world ecs;
    ecs.import<flecs::components::meta>();
    ecs.import<Rendering>();
    ecs.import<Input>();
    ecs.import<flecs::dash>();
    ecs.import<flecs::systems::civetweb>();
    ecs.entity().set<flecs::dash::Server>({9090});

    return ecs;
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.prefab("WindowSize").set<Extent2D>({1000, 800});

    auto application = ecs.entity("Minecraft2").add_instanceof(windowSize);
    Rendering::regApplication(application);

    guiSubmodule<TypeTable>(ecs, application, "", editorGuiSubmodule);
    regGuiComponent<Position3D>(application, GuiComponentType::DragFloat3);
    regGuiComponent<Path>(application, GuiComponentType::Text);

    auto mesh = ecs.entity("CubeMesh").set<Path>({"cube.obj"});
    Rendering::regMesh(application, mesh);

    auto texture = ecs.entity("CubeTexture").set<Path>({"field.jpg"});
    Rendering::regTexture(application, texture);

    auto camera = ecs.entity("Viewport").
            add_instanceof(windowSize).
            set<Position3D>({-2, 0, 1}).
            set<Distance>({50.f}).
            add<Input::Controllable>();

    Rendering::regViewport(application, camera);
    Rendering::regPointLight(application, camera);

    auto cube = ecs.entity("Cube").
            add_instanceof(mesh).
            set(DiffuseTexture{texture}).
            set<Position3D>({0, 0, 0}).
            set<Scale3D>({5.f, 0.2f, 5.f});
    Rendering::regModel(application, cube);
    cube.add_instanceof(camera);

    auto ball = ecs.entity("Ball").
            set<Path>({"sphere.obj"}).
            set<Position3D>({0, 1, 0}).
            set<Scale3D>({0.01, 0.01, 0.01}).
            set<DiffuseColor>({0.8, 0, 0});
    Rendering::regMesh(application, ball);
    Rendering::regModel(application, ball);
    ball.add_instanceof(camera);

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
