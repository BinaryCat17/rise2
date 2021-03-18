#include <rise/rendering/llgl/module.hpp>
#include <rise/rendering/editor.hpp>
#include <rise/input/module.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/util/flecs_os.hpp>
#include <rise/editor/gui.hpp>

using namespace rise;

flecs::world initWorld() {
    stdcpp_set_os_api();

    flecs::world ecs;
    ecs.import<rise::rendering::LLGLModule>();
    ecs.import<rise::editor::Module>();
    ecs.import<rise::rendering::EditorComponents>();
    ecs.import<rise::input::Module>();
    ecs.import<flecs::dash>();
    ecs.import<flecs::systems::civetweb>();
    ecs.entity().set<flecs::dash::Server>({9090});

    return ecs;
}

int main() {
    auto ecs = initWorld();

    auto application = ecs.entity("Minecraft2").
            set<rendering::Extent2D>({1000, 800}).
            add<rendering::LLGLApplication>();

    guiSubmodule(ecs, application, editor::guiSubmodule);

    auto mesh = ecs.entity("CubeMesh").
            add_instanceof(application).
            set<rendering::Path>({"cube.obj"}).
            add<rendering::Mesh>();

    auto texture = ecs.entity("CubeTexture").
            add_instanceof(application).
            set<rendering::Path>({"field.jpg"}).
            add<rendering::Texture>();

    auto camera = ecs.entity("Viewport").
            add_instanceof(application).
            set<rendering::Position3D>({-2, 0, 1}).
            set<rendering::Distance>({50.f}).
            add<input::Controllable>().
            add<rendering::Viewport>().
            add<rendering::PointLight>();
    ecs.entity("Cube").
            add_instanceof(application).
            add_instanceof(mesh).
            add_instanceof(camera).
            set(rendering::DiffuseTexture{texture}).
            set<rendering::Position3D>({0, 0, 0}).
            set<rendering::Scale3D>({5.f, 0.2f, 5.f}).
            add<rendering::Model>();

    ecs.entity("Ball").
            add_instanceof(application).
            add_instanceof(camera).
            set<rendering::Path>({"sphere.obj"}).
            set<rendering::Position3D>({0, 1, 0}).
            set<rendering::Scale3D>({0.01, 0.01, 0.01}).
            set<rendering::DiffuseColor>({0.8, 0, 0}).
            add<rendering::Mesh>().
            add<rendering::Model>();

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
