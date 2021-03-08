#pragma once
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct MeshResource {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
    };

    MeshResource loadMeshFromDisk(LLGL::RenderSystem *renderer, std::string const &path,
            LLGL::VertexFormat const &format);
}

