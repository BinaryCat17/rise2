#include <rise/systems/rendering/module.hpp>
#include <rise/systems/input/module.hpp>
#include <rise/components/rendering/module.hpp>
#include <rise/components/rendering/imgui.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/util/flecs_os.hpp>
#include <iostream>

using namespace rise::systems;
using namespace rise::components::rendering;

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

void guiSystem(flecs::entity, GuiContext gui) {
    ImGui::SetCurrentContext(gui.context);
    ImGui::ShowDemoWindow();
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.prefab("WindowSize").set<Extent2D>({1600, 1000});
    auto application = ecs.entity("Minecraft2").add_instanceof(windowSize);
    guiSubmodule(ecs, application, guiSystem);
    Rendering::regApplication(application);

    auto mesh = ecs.entity("CubeMesh").set<Path>({"cube.obj"});
    Rendering::regMesh(application, mesh);

    auto texture = ecs.entity("CubeTexture").set<Path>({"paper.jpg"});
    Rendering::regTexture(application, texture);

    auto camera = ecs.entity("Viewport").
            add_instanceof(windowSize).
            set<Position3D>({-2, 0, 1}).
            set<Distance>({15.f}).
            add<Input::Controllable>();

    Rendering::regViewport(application, camera);
    Rendering::regPointLight(application, camera);

    auto cube = ecs.entity("Cube").
            add_instanceof(mesh).
            add_instanceof(texture).
            set<Position3D>({0, 0, 0});
    Rendering::regModel(application, cube);
    cube.add_instanceof(camera);

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
