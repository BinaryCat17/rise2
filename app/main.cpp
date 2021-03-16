#include <rise/systems/rendering/module.hpp>
#include <rise/systems/input/module.hpp>
#include <rise/components/rendering/imgui.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/util/flecs_os.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <iostream>
#include <memory>

using namespace rise;
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

template<typename T>
void regDragFloat3(flecs::entity e, std::vector<char const *> &missing, T *val) {
    if (val && e.owns<T>()) {
        ImGui::DragFloat3(flecs::__meta__<T>::name(), reinterpret_cast<float *>(val));
        e.set<T>(*val);
    } else {
        missing.push_back(flecs::__meta__<T>::name());
    }
}

template<typename T>
void regDragFloat2(flecs::entity e, std::vector<char const *> &missing, T *val) {
    if (val && e.owns<T>()) {
        ImGui::DragFloat2(flecs::__meta__<T>::name(), reinterpret_cast<float *>(val));
        e.set<T>(*val);
    } else {
        missing.push_back(flecs::__meta__<T>::name());
    }
}

template<typename T>
void regDragFloat(flecs::entity e, std::vector<char const *> &missing, T *val) {
    if (val && e.owns<T>()) {
        ImGui::DragFloat(flecs::__meta__<T>::name(), reinterpret_cast<float *>(val));
        e.set<T>(*val);
    } else {
        missing.push_back(flecs::__meta__<T>::name());
    }
}

void regTextInput(flecs::entity e, std::vector<char const *> &missing, Path *path) {
    if (e.owns<Path>() && path) {
        std::string str = path->file;
        ImGui::InputText("Path", &str);
        if (str != path->file) {
            e.set<Path>(Path{str});
        }
    } else {
        missing.push_back("Path");
    }
}

struct SelectedMissingItem {
    int pos;
};

enum class GuiComponentType {
    DragFloat,
    DragFloat3,
    DragFloat2,
    Text, // std string
};

flecs::map<flecs::entity_t, GuiComponentType> typeTable;

void guiSystem(flecs::entity e, GuiContext gui,
        SelectedMissingItem *misPos, Position3D *position3D,
        Rotation3D *rotation3D, Scale3D *scale3D, Extent3D *extent3D, Position2D *position2D,
        Rotation2D *rotation2D, Scale2D *scale2D, Extent2D *extent2D, DiffuseColor *color,
        Path *path, Intensity *intensity, Distance *distance) {
    ImGui::SetCurrentContext(gui.context);

    if (!misPos) {
        e.set<SelectedMissingItem>({-1});
        return;
    }

    if (!e.name().empty() && ImGui::TreeNode(e.name().c_str())) {
        std::vector<char const *> missingComponents;

        regDragFloat3(e, missingComponents, position3D);
        regDragFloat3(e, missingComponents, rotation3D);
        regDragFloat3(e, missingComponents, scale3D);
        regDragFloat3(e, missingComponents, extent3D);
        regDragFloat2(e, missingComponents, position2D);
        regDragFloat2(e, missingComponents, rotation2D);
        regDragFloat2(e, missingComponents, scale2D);
        regDragFloat2(e, missingComponents, extent2D);
        regDragFloat3(e, missingComponents, color);
        regTextInput(e, missingComponents, path);
        regDragFloat(e, missingComponents, intensity);
        regDragFloat(e, missingComponents, distance);

        ImGui::ListBox("Add component", &misPos->pos, missingComponents.data(),
                static_cast<int>(missingComponents.size()), 4);

        if (misPos->pos != -1) {
            auto query = std::string("rise.components.rendering.") +
                    missingComponents[misPos->pos];
            auto type = e.world().lookup(query);
            misPos->pos = -1;
        }

        ImGui::NewLine();

        ImGui::TreePop();
    }
}

int main() {
    auto ecs = initWorld();

    auto windowSize = ecs.prefab("WindowSize").set<Extent2D>({1000, 800});
    auto application = ecs.entity("Minecraft2").add_instanceof(windowSize);
    guiSubmodule<SelectedMissingItem *, Position3D *, Rotation3D *,
            Scale3D *, Extent3D *, Position2D *, Rotation2D *, Scale2D *, Extent2D *,
            DiffuseColor *, Path *, Intensity *, Distance *
    >(ecs, application, "", guiSystem);

    Rendering::regApplication(application);

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
