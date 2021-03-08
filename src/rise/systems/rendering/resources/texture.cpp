#include "texture.hpp"
#include "util.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

namespace rise::systems::rendering {
    TextureResource loadTextureFromDisk(LLGL::RenderSystem *renderer, std::string const &path) {
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char *imageBuffer = stbi_load(path.c_str(),
                &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to load image from file: " + path);

        auto texture = createTextureFromData(renderer,
                (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB),
                imageBuffer, texWidth, texHeight);

        stbi_image_free(imageBuffer);

        return {texture};
    }
}