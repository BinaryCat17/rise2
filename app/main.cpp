#include <rise/rendering/llgl/module.hpp>
#include <rise/rendering/editor.hpp>
#include <rise/input/module.hpp>
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

    return ecs;
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.entity("WindowSize").set<rendering::Extent2D>({1600, 1000});

    auto application = ecs.entity("Minecraft2").
            add_instanceof(windowSize).
            add<rendering::LLGLApplication>();

    guiSubmodule(ecs, application, editor::guiSubmodule);

    auto mesh = ecs.entity("CubeMesh").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"cube.obj"}).
            add<rendering::Mesh>();

    auto texture = ecs.entity("CubeTexture").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"field.jpg"}).
            add<rendering::Texture>();

    auto camera = ecs.entity("Viewport").
            set<rendering::RegTo>({application}).
            add_instanceof(windowSize).
            set<rendering::Position3D>({-2, 2, 1}).
            set<rendering::Distance>({50.f}).
            add<input::Controllable>().
            add<rendering::Viewport>().
            add<rendering::PointLight>();

    ecs.entity("Cube").
            set<rendering::RenderTo>({camera}).
            add_instanceof(mesh).
            set<rendering::DiffuseTexture>({texture}).
            set<rendering::Position3D>({0, 0, 0}).
            set<rendering::Scale3D>({5.f, 0.2f, 5.f}).
            add<rendering::Model>();

    ecs.entity("Ball").
            set<rendering::RenderTo>({camera}).
            set<rendering::Path>({"sphere.obj"}).
            set<rendering::Position3D>({0, 1, 0}).
            set<rendering::Scale3D>({0.01, 0.01, 0.01}).
            set<rendering::DiffuseColor>({0.8, 0, 0}).
            add<rendering::Mesh>().
            add<rendering::Model>();

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
