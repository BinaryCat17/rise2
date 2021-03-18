#include "texture.hpp"
#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <iostream>

namespace rise::rendering {
    void updateTexture(flecs::entity, CoreState &core, TextureRes &texture, Path const &path) {
        int texWidth = 0, texHeight = 0, texComponents = 0;

        auto file = core.path + "/textures/" + path.file;

        unsigned char *imageBuffer = stbi_load(file.c_str(),
                &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer) {
            std::cerr << "failed to load image from file: " << file << std::endl;
            return;
        }

        if (texture.val) {
            core.renderer->Release(*texture.val);
        }

        texture.val = createTextureFromData(core.renderer.get(),
                (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB),
                imageBuffer, texWidth, texHeight);

        stbi_image_free(imageBuffer);
    }
}