#include <rise/rendering/llgl/module.hpp>
#include <rise/rendering/editor.hpp>
#include <rise/rendering/glm.hpp>
#include <rise/input/module.hpp>
#include <rise/util/flecs_os.hpp>
#include <rise/editor/gui.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace rise;

struct RotateAroundCenter {
    bool enabled = true;
};

void rotateAroundCenter(flecs::entity e, RotateAroundCenter v, rendering::Position3D pos,
        rendering::Rotation3D rotation) {
    if (v.enabled) {
        auto rt = glm::rotate(glm::vec2(pos.x, pos.z), e.delta_time());
        e.set<rendering::Position3D>({rt.x, pos.y, rt.y});
    }
}

flecs::world initWorld() {
    stdcpp_set_os_api();

    flecs::world ecs;
    ecs.import<rise::rendering::Module>();
    ecs.import<rise::input::Module>();
    ecs.import<rise::rendering::LLGLModule>();
    ecs.import<rise::editor::Module>();
    ecs.import<rise::rendering::EditorComponents>();
    ecs.component<RotateAroundCenter>("RotateAroundCenter");
    editor::regGuiComponent<RotateAroundCenter>(ecs, editor::GuiComponentType::BoolFlag);
    ecs.system<const RotateAroundCenter, rendering::Position3D,
            rendering::Rotation3D>("rotateBalls").each(rotateAroundCenter);

    return ecs;
}

int main() {
    auto ecs = initWorld();


    auto windowSize = ecs.entity("WindowSize").set<rendering::Extent2D>({1600, 1000});

    auto application = ecs.entity("Minecraft2").
            add_instanceof(windowSize).
            add<rendering::LLGLApplication>();


    rendering::guiSubmodule<const rendering::RenderTo, rendering::Position3D, rendering::Rotation3D,
            rendering::Scale3D>(ecs, "drawImGuizmo", application, editor::imGuizmoSubmodule);
    rendering::guiSubmodule(ecs, "drawComponents", application, editor::guiSubmodule);

    auto camera = ecs.entity("Viewport").
            set<rendering::RegTo>({application}).
            add_instanceof(windowSize).
            set<rendering::Position3D>({-10, 14, 14}).
            set<rendering::Distance>({50.f}).
            add<input::Controllable>().
            add<rendering::Viewport>();

    auto mesh = ecs.entity("GroundMesh").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"cube.obj"}).
            add<rendering::Mesh>();

    auto panelAlbedo = ecs.entity("PanelAlbedoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"panel/albedo.png"}).
            add<rendering::Texture>();

    auto panelRoughness = ecs.entity("PanelRoughnessTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ground/roughness.png"}).
            add<rendering::Texture>();

    auto panelAo = ecs.entity("PanelAoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"panel/ao.png"}).
            add<rendering::Texture>();

    auto panelMetallic = ecs.entity("PanelMetallicTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"panel/metallic.png"}).
            add<rendering::Texture>();

    auto panel = ecs.entity("Panel").
            set<rendering::RegTo>({application}).
            add_instanceof(mesh).
            set<rendering::AlbedoTexture>({panelAlbedo}).
            set<rendering::RoughnessTexture>({panelRoughness}).
            set<rendering::MetallicTexture>({panelMetallic}).
            set<rendering::AoTexture>({panelAo}).
            set<rendering::Albedo>({0.2, 0.2, 0.2}).
            set<rendering::Position3D>({0, 0, 0}).
            set<rendering::Scale3D>({10.f, 0.2f, 10.f}).
            set<rendering::Albedo>({0.220, 0.280, 0.330}).
            add<rendering::Material>();

    ecs.entity("Panel1").
            add_instanceof(panel).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({0, -1, -10}).
            add<rendering::Model>();

    ecs.entity("Panel2").
            add_instanceof(panel).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({0, 0, 0}).
            add<rendering::Model>();

    ecs.entity("Panel3").
            add_instanceof(panel).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({0, 1, 10}).
            add<rendering::Model>();

    auto groundAlbedo = ecs.entity("GroundAlbedoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ground/albedo.png"}).
            add<rendering::Texture>();

    auto groundRoughness = ecs.entity("GroundRoughnessTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ground/roughness.png"}).
            add<rendering::Texture>();

    auto groundAo = ecs.entity("GroundAoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ground/ao.png"}).
            add<rendering::Texture>();

    auto groundMetallic = ecs.entity("GroundMetallicTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ground/metallic.png"}).
            add<rendering::Texture>();

    auto cube1 = ecs.entity("CubePreset1").
            set<rendering::RegTo>({application}).
            add_instanceof(mesh).
            set<rendering::AlbedoTexture>({groundAlbedo}).
            set<rendering::RoughnessTexture>({groundRoughness}).
            set<rendering::MetallicTexture>({groundMetallic}).
            set<rendering::AoTexture>({groundAo}).
            set<rendering::Albedo>({0.2, 0.2, 0.2}).
            set<rendering::Scale3D>({10.f, 0.2f, 10.f}).
            add<rendering::Material>();

    ecs.entity("Cube1").
            add_instanceof(cube1).
            set<rendering::Position3D>({10, -1, -10}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    ecs.entity("Cube2").
            add_instanceof(cube1).
            set<rendering::Position3D>({10, 0, 0}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    ecs.entity("Cube3").
            add_instanceof(cube1).
            set<rendering::Position3D>({10, 1, 10}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    auto grassAlbedo = ecs.entity("GrassAlbedoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"grass/albedo.png"}).
            add<rendering::Texture>();

    auto grassRoughness = ecs.entity("GrassRoughnessTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"grass/roughness.png"}).
            add<rendering::Texture>();

    auto grassAo = ecs.entity("GrassAoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"grass/ao.png"}).
            add<rendering::Texture>();

    auto grassMetallic = ecs.entity("GrassMetallicTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"grass/metallic.png"}).
            add<rendering::Texture>();

    auto cube2 = ecs.entity("CubePreset2").
            set<rendering::RegTo>({application}).
            add_instanceof(mesh).
            set<rendering::AlbedoTexture>({grassAlbedo}).
            set<rendering::RoughnessTexture>({grassRoughness}).
            set<rendering::MetallicTexture>({grassMetallic}).
            set<rendering::AoTexture>({grassAo}).
            set<rendering::Albedo>({0.2, 0.2, 0.2}).
            set<rendering::Scale3D>({10.f, 0.2f, 10.f}).
            add<rendering::Material>();

    ecs.entity("Ave1").
            add_instanceof(cube2).
            set<rendering::Position3D>({-10, -1, -10}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    ecs.entity("Ave2").
            add_instanceof(cube2).
            set<rendering::Position3D>({-10, 0, 0}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    ecs.entity("Ave3").
            add_instanceof(cube2).
            set<rendering::Position3D>({-10, 1, 10}).
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    ecs.entity("Light").
            set<rendering::RenderTo>({camera}).
            add_instanceof(mesh).
            set<rendering::Albedo>({1.0, 0.0, 0.0}).
            set<rendering::Position3D>({20, 100, 20}).
            set<rendering::Scale3D>({0.5f, 0.5f, 0.5f}).
            set<rendering::Distance>({3000.f}).
            set<rendering::Intensity>({0.01f}).
            add<rendering::PointLight>().
            add<rendering::Model>().
            add<RotateAroundCenter>();

    ecs.entity("Light2").
            set<rendering::RenderTo>({camera}).
            add_instanceof(mesh).
            set<rendering::Albedo>({1.0, 0.0, 0.0}).
            set<rendering::Position3D>({-15, 80, -15}).
            set<rendering::Scale3D>({0.5f, 0.5f, 0.5f}).
            set<rendering::Distance>({3000.f}).
            set<rendering::Intensity>({0.01f}).
            add<rendering::PointLight>().
            add<rendering::Model>();

    auto robotAlbedo = ecs.entity("RobotAlbedoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"robot/albedo.png"}).
            add<rendering::Texture>();

    auto robotMetallic = ecs.entity("RobotMetallicTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"robot/metallic.png"}).
            add<rendering::Texture>();

    auto robotAo = ecs.entity("RobotAoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"robot/ao.png"}).
            add<rendering::Texture>();

    ecs.entity("Robot").
            set<rendering::RenderTo>({camera}).
            set<rendering::Path>({"robot.obj"}).
            set<rendering::Position3D>({0, 0, 0}).
            set<rendering::Scale3D>({0.5, 0.5, 0.5}).
            set<rendering::Ao>({20.0}).
            set<rendering::Roughness>({0.5}).
            set<rendering::Albedo>({0.5, 0.5, 0.5}).
            set<rendering::Metallic>({0.340}).
            set<rendering::AlbedoTexture>({robotAlbedo}).
            set<rendering::MetallicTexture>({robotMetallic}).
            set<rendering::AoTexture>({robotAo}).
            add<rendering::Material>().
            add<rendering::Mesh>().
            add<rendering::Shadow>().
            add<rendering::Model>();

    auto ballAlbedo = ecs.entity("BallAlbedoTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ball/albedo.png"}).
            add<rendering::Texture>();

    auto ballMetallic = ecs.entity("BallMetallicTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ball/metallic.png"}).
            add<rendering::Texture>();

    auto ballRoughness = ecs.entity("BallRoughnessTex").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"ball/roughness.png"}).
            add<rendering::Texture>();

    auto ball = ecs.entity("Ball").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"sphere.obj"}).
            set<rendering::Scale3D>({0.5, 0.5, 0.5}).
            set<rendering::Albedo>({0.2, 0.2, 0.2}).
            set<rendering::AlbedoTexture>({ballAlbedo}).
            set<rendering::MetallicTexture>({ballMetallic}).
            set<rendering::RoughnessTexture>({ballRoughness}).
            add<rendering::Mesh>().
            add<rendering::Material>();

    ecs.entity("Ball1").
            add_instanceof(ball).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({0, 2, -5}).
            add<RotateAroundCenter>().
            add<rendering::Model>().
            add<rendering::Shadow>();

    ecs.entity("Ball2").
            add_instanceof(ball).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({0, 3, 5}).
            add<RotateAroundCenter>().
            add<rendering::Model>().
            add<rendering::Shadow>();

    ecs.entity("Ball3").
            add_instanceof(ball).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({-10, 4, 0}).
            add<RotateAroundCenter>().
            add<rendering::Model>().
            add<rendering::Shadow>();

    ecs.entity("Ball4").
            add_instanceof(ball).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>({3, 8, 0}).
            add<RotateAroundCenter>().
            add<rendering::Model>().
            add<rendering::Shadow>();

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
