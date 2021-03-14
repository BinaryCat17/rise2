#pragma once
#include <imgui.h>
#include "rise/util/ecs.hpp"

namespace rise::components::rendering {
    struct GuiContext {
        ImGuiContext* context;
    };

    template<typename... Types, typename F>
    void guiSubmodule(flecs::world &ecs, flecs::entity app, F &&f) {
        auto expr = app.name() + ":rise.components.rendering.GuiContext,"
                                 "OWNED:rise.components.rendering.GuiContext";
        ecs.system<rendering::GuiContext, Types...>("drawGui", expr.c_str()).
                kind(flecs::PreStore).each(f);
    }
}