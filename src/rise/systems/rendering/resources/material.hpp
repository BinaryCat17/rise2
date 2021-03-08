#pragma once
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct MaterialResource {
        LLGL::Buffer *uniform = nullptr;
    };
}
