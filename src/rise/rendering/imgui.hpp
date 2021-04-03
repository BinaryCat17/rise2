#pragma once

#include <imgui.h>
#include <flecs.h>
#include "module.hpp"

namespace rise::rendering {
    struct GuiContext {
        ImGuiContext *context;
    };

    template<typename... Types, typename F>
    void guiSubmodule(flecs::world &ecs, std::string const &name, flecs::entity app, F &&f,
            std::string const &query = {}) {
        auto expr = app.name().c_str() + std::string(":rise.rendering.GuiContext,") + query;
        ecs.system<rendering::RegTo, Types...>(name.c_str(), expr.c_str()).
                kind(flecs::PreStore).each(f);
    }
}