#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

namespace rise {
    struct Mesh {
        LLGL::VertexFormat format = {};
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        uint32_t numIndices = 0;
    };

    LLGL::VertexFormat getVertexFormat();

    Mesh loadMesh(LLGL::RenderSystem *renderer, std::string const &path);

    void drawMesh(LLGL::CommandBuffer *cmdBuf, Mesh& mesh);
}
