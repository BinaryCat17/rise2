#pragma once
#include <entt/entt.hpp>

namespace rise {
    struct RenderSystem {
        static void init(entt::registry &r);

        static bool update(entt::registry &r);

        static void destroy(entt::registry &r);
    };
}