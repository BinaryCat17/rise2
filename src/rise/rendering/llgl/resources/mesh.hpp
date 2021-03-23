#pragma once

#include <flecs.h>
#include <LLGL/LLGL.h>

namespace rise::rendering {
    struct MeshRes {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
        unsigned numVertices = 0;
    };

    void importMesh(flecs::world& ecs);
}

