#pragma once

#include <imgui.h>
#include <flecs.h>
#include "module.hpp"

namespace rise::rendering {
    struct GuiContext {
        ImGuiContext *context;
    };

    template<typename... Types, typename F>
    void guiSubmodule(flecs::world &ecs, flecs::entity app, F &&f, std::string const &query = {}) {
        auto expr = app.name() + ":rise.rendering.GuiContext," + query;
        ecs.system<rendering::RegTo, Types...>("drawGui", expr.c_str()).
                kind(flecs::PreStore).each(f);
    }
}