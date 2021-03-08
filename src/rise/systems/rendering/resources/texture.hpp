#pragma once
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct TextureResource {
        LLGL::Texture *texture = nullptr;
    };

    TextureResource loadTextureFromDisk(LLGL::RenderSystem *renderer, std::string const &path);
}
