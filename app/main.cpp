#include <rise/rendering/llgl/module.hpp>
#include <rise/rendering/editor.hpp>
#include <rise/rendering/glm.hpp>
#include <rise/input/module.hpp>
#include <rise/util/flecs_os.hpp>
#include <rise/physics/module.hpp>
#include <rise/editor/gui.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace rise;

struct RotateAroundCenter {
    bool enabled = true;
};

void rotateAroundCenter(flecs::entity e, RotateAroundCenter v, rendering::Position3D pos) {
    if (v.enabled) {
        auto rt = glm::rotate(glm::vec2(pos.x, pos.z), e.delta_time() * 2);
        e.set<rendering::Position3D>({rt.x, pos.y, rt.y});
    }
}

flecs::world initWorld() {
    stdcpp_set_os_api();

    flecs::world ecs;
    ecs.import<rise::rendering::Module>();
    ecs.import<rise::input::Module>();
    ecs.import<rise::rendering::LLGLModule>();
    ecs.import<rise::physics::Module>();
    ecs.import<rise::editor::Module>();
    ecs.import<rise::rendering::EditorComponents>();
    ecs.component<RotateAroundCenter>("RotateAroundCenter");
    editor::regGuiComponent<RotateAroundCenter>(ecs, editor::GuiComponentType::BoolFlag);
    ecs.system<const RotateAroundCenter, rendering::Position3D>("rotateBalls").
            each(rotateAroundCenter);

    return ecs;
}

flecs::entity makeModelPreset(flecs::world &ecs, flecs::entity application, std::string const &name,
        flecs::entity mesh, std::string const &texturesFolder,
        rendering::Scale3D scale = {1.0f, 1.0f, 1.0f},
        rendering::Albedo albedo = {1.0, 1.0, 1.0}) {

    auto panelAlbedo = ecs.entity().
            set<rendering::RegTo>({application}).
            set<rendering::Path>({texturesFolder + "/albedo.png"}).
            add<rendering::Texture>();

    auto panelRoughness = ecs.entity().
            set<rendering::RegTo>({application}).
            set<rendering::Path>({texturesFolder + "/roughness.png"}).
            add<rendering::Texture>();

    auto panelAo = ecs.entity().
            set<rendering::RegTo>({application}).
            set<rendering::Path>({texturesFolder + "/ao.png"}).
            add<rendering::Texture>();

    auto panelMetallic = ecs.entity().
            set<rendering::RegTo>({application}).
            set<rendering::Path>({texturesFolder + "/metallic.png"}).
            add<rendering::Texture>();

    return ecs.entity(name.c_str()).
            set<rendering::RegTo>({application}).
            add_instanceof(mesh).
            set<rendering::AlbedoTexture>({panelAlbedo}).
            set<rendering::RoughnessTexture>({panelRoughness}).
            set<rendering::MetallicTexture>({panelMetallic}).
            set<rendering::AoTexture>({panelAo}).
            set<rendering::Scale3D>(scale).
            set<rendering::Albedo>(albedo).
            add<rendering::Material>();
}

flecs::entity makeModelChild(flecs::world &ecs, flecs::entity parent, flecs::entity camera,
        std::string const &name, rendering::Position3D position) {
    return ecs.entity(name.c_str()).
            add_instanceof(parent).
            set<rendering::RenderTo>({camera}).
            set<rendering::Position3D>(position).
            add<rendering::Model>();
}

flecs::entity makeLight(flecs::world &ecs, std::string const &name,
        flecs::entity mesh, flecs::entity camera, rendering::Position3D position) {
    return ecs.entity(name.c_str()).
            set<rendering::RenderTo>({camera}).
            add_instanceof(mesh).
            set<rendering::Position3D>({position}).
            set<rendering::Albedo>({1.0, 0.0, 0.0}).
            set<rendering::Scale3D>({0.5f, 0.5f, 0.5f}).
            set<rendering::Distance>({3000.f}).
            set<rendering::Intensity>({0.01f}).
            add<rendering::PointLight>().
            add<rendering::Model>();
}

auto boxCollision(flecs::entity e, physics::BodyType type) {
    e.set<physics::PhysicBody>({type});
    auto size = *e.get<rendering::Scale3D>();
    e.set<physics::BoxCollision>({{size.x / 2, size.y / 2, size.z / 2}});
    return e;
}

auto sphereCollision(flecs::entity e, physics::BodyType type) {
    e.set<physics::PhysicBody>({type});
    auto size = *e.get<rendering::Scale3D>();
    e.set<physics::SphereCollision>({size.x * 2});
    return e;
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.entity("WindowSize").set<rendering::Extent2D>({1000, 800});

    auto application = ecs.entity("Minecraft2").
            add_instanceof(windowSize).
            add<rendering::LLGLApplication>();

    rendering::guiSubmodule<const rendering::RenderTo, rendering::Position3D, rendering::Rotation3D,
            rendering::Scale3D>(ecs, "drawImGuizmo", application, editor::imGuizmoSubmodule);
    rendering::guiSubmodule(ecs, "drawComponents", application, editor::guiSubmodule);

    auto camera = ecs.entity("Viewport").
            set<rendering::RegTo>({application}).
            add_instanceof(windowSize).
            set<rendering::Position3D>({-15, 20, -10}).
            set<rendering::Distance>({50.f}).
            add<input::Controllable>().
            add<rendering::Viewport>();

    auto cube = ecs.entity("CubeMesh").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"cube.obj"}).
            add<rendering::Mesh>();

    auto panel = makeModelPreset(ecs, application, "Panel", cube, "panel",
            {10.0, 0.2, 10.0}, {0.220, 0.280, 0.330});

    boxCollision(makeModelChild(ecs, panel, camera, "Panel1", {0, -1, -10}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, panel, camera, "Panel2", {0, 0, 0}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, panel, camera, "Panel3", {0, 1.290, 10}),
            physics::BodyType::STATIC).set<rendering::Rotation3D>({176.725, 0.0, 0.0});

    auto title = makeModelPreset(ecs, application, "Title", cube, "title",
            {10.0, 0.2, 10.0}, {0.550, 0.420, 0.290});

    boxCollision(makeModelChild(ecs, title, camera, "Title1", {-10, 0.406, 1.597}).
            set<rendering::Scale3D>({3.0, 0.2, 7.0}).
            set<rendering::Rotation3D>({170, 0.0, 0.0}).
            add<rendering::Shadow>(), physics::BodyType::STATIC);

    boxCollision(makeModelChild(ecs, title, camera, "Title2", {10, 0.406, 1.597}).
            set<rendering::Scale3D>({3.0, 0.2, 7.0}).
            set<rendering::Rotation3D>({170, 0.0, 0.0}).
            add<rendering::Shadow>(), physics::BodyType::STATIC);

    boxCollision(makeModelChild(ecs, title, camera, "Title3", {-10, -0.510, -8.449}).
            set<rendering::Scale3D>({3.0, 0.2, 7.0}).
            set<rendering::Rotation3D>({170, 0.0, 0.0}).
            add<rendering::Shadow>(), physics::BodyType::STATIC);

    boxCollision(makeModelChild(ecs, title, camera, "Title4", {10, -0.510, -8.449}).
            set<rendering::Scale3D>({3.0, 0.2, 7.0}).
            set<rendering::Rotation3D>({170, 0.0, 0.0}).
            add<rendering::Shadow>(), physics::BodyType::STATIC);

    auto ground = makeModelPreset(ecs, application, "Ground", cube, "ground",
            {10.0, 0.2, 10.0}, {0.2, 0.2, 0.2});

    boxCollision(makeModelChild(ecs, ground, camera, "Ground1", {10, -1, -10}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, ground, camera, "Ground2", {10, 0, 0}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, ground, camera, "Ground3", {10, 1.290, 10}),
            physics::BodyType::STATIC).set<rendering::Rotation3D>({176.725, 0.0, 0.0});

    auto grass = makeModelPreset(ecs, application, "Grass", cube, "grass",
            {10.0, 0.2, 10.0}, {0.2, 0.2, 0.2});

    boxCollision(makeModelChild(ecs, grass, camera, "Grass1", {-10, -1, -10}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, grass, camera, "Grass2", {-10, 0, 0}),
            physics::BodyType::STATIC);
    boxCollision(makeModelChild(ecs, grass, camera, "Grass3", {-10, 1.290, 10}),
            physics::BodyType::STATIC).set<rendering::Rotation3D>({176.725, 0.0, 0.0});

    makeLight(ecs, "RotationLight", cube, camera, {20, 100, 20}).set<RotateAroundCenter>({true});
    makeLight(ecs, "StaticLight", cube, camera, {40, 100, 40});

    auto robotMesh = ecs.entity().
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"robot.obj"}).
            add<rendering::Mesh>();

    auto robot = makeModelPreset(ecs, application, "Robot", robotMesh, "robot",
            {0.5, 0.5, 0.5}, {0.5, 0.5, 0.5});

    robot.set<rendering::Ao>({20.0}).
            set<rendering::Roughness>({0.5}).
            set<rendering::Albedo>({0.5, 0.5, 0.5}).
            set<rendering::Metallic>({0.340}).
            add<rendering::Shadow>().
            set<rendering::RenderTo>({camera}).
            add<rendering::Model>();

    auto ballMesh = ecs.entity("BallMesh").
            set<rendering::RegTo>({application}).
            set<rendering::Path>({"sphere.obj"}).
            add<rendering::Mesh>();

    auto ball = makeModelPreset(ecs, application, "Ball", ballMesh, "ball",
            {0.5, 0.5, 0.5}, {0.2, 0.2, 0.2});

    sphereCollision(makeModelChild(ecs, ball, camera, "Ball1", {0, 2, -8}).
            add<rendering::Shadow>().add<RotateAroundCenter>(), physics::BodyType::STATIC);
    sphereCollision(makeModelChild(ecs, ball, camera, "Ball2", {0, 3, 8}).
            add<rendering::Shadow>().add<RotateAroundCenter>(), physics::BodyType::STATIC);
    sphereCollision(makeModelChild(ecs, ball, camera, "Ball3", {-12, 4, 0}).
            add<rendering::Shadow>().add<RotateAroundCenter>(), physics::BodyType::STATIC);
    sphereCollision(makeModelChild(ecs, ball, camera, "Ball4", {5, 8, 0}).
            add<rendering::Shadow>().add<RotateAroundCenter>(), physics::BodyType::STATIC);
    sphereCollision(makeModelChild(ecs, ball, camera, "Ball5", {-10, 4, 10}).
            add<rendering::Shadow>(), physics::BodyType::DYNAMIC).
            set<physics::Velocity>({0.0, 0.0, -5.0}).set<physics::Mass>({100.0});
    sphereCollision(makeModelChild(ecs, ball, camera, "Ball6", {10, 4, 10}).
            add<rendering::Shadow>(), physics::BodyType::DYNAMIC).
            set<physics::Velocity>({0.0, 0.0, -5.0}).set<physics::Mass>({100.0});

    auto stone = makeModelPreset(ecs, application, "Stone", cube, "brick",
            {2.0, 2.0, 2.0}, {0.5, 0.5, 0.5});

    for (int i = 0; i != 16; ++i) {
        for (int j = 0; j != 5; ++j) {
            boxCollision(makeModelChild(ecs, stone, camera,
                    "StoneR" + std::to_string(i) + std::to_string(j), {
                            static_cast<float>(-14 + i * 2),
                            static_cast<float>(0.1 + j * 2),
                            static_cast<float>(-12)}).
                    add<rendering::Shadow>(), physics::BodyType::DYNAMIC);
        }
    }

    ecs.set_target_fps(60);
    while (ecs.progress()) {}
}
