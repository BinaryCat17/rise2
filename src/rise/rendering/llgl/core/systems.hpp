#pragma once
#include "state.hpp"

namespace rise::rendering {
    void prepareRender(flecs::entity, CoreState &core);

    void submitRender(flecs::entity, CoreState &core);
}
