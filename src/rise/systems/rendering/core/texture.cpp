#include "texture.hpp"
#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

namespace rise::systems::rendering {
    void updateTexture(flecs::entity, RenderSystem const &renderer, Texture &texture,
            Path const &path) {
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char *imageBuffer = stbi_load(path.file,
                &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to load image from file: " + std::string(path.file));

        if (texture) {
            renderer->Release(*texture);
        }

        texture = createTextureFromData(renderer.get(),
                (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB),
                imageBuffer, texWidth, texHeight);

        stbi_image_free(imageBuffer);
    }
}