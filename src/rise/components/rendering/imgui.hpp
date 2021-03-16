#pragma once
#include <imgui.h>
#include "rise/util/ecs.hpp"

namespace rise::components::rendering {
    struct GuiContext {
        ImGuiContext* context;
    };

    template<typename... Types, typename F>
    void guiSubmodule(flecs::world &ecs, flecs::entity app, std::string const& query, F &&f) {
        auto expr = app.name() + ":rise.components.rendering.GuiContext," + query;
        ecs.system<rendering::GuiContext, Types...>("drawGui", expr.c_str()).
                kind(flecs::PreStore).each(f);
    }
}