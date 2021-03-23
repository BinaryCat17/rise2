#define STB_IMAGE_IMPLEMENTATION
#include "resources.hpp"
#include "stb_image.h"
#include <iostream>

namespace rise::rendering {
    void regTexture(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({});
        e.set<TextureRes>({});
    }

    void unregTexture(flecs::entity e) {
        auto &core = *e.get<RegTo>()->e.get<CoreState>();
        auto &texture = *e.get_mut<TextureRes>();
        core.renderer->Release(*e.get<TextureRes>()->val);
        e.remove<TextureRes>();
        e.mut();
    }

    void updateTexture(flecs::entity, RegTo state, TextureRes &texture, Path const &path) {
        auto& core = *state.e.get<CoreState>();

        auto file = core.root + "/textures/" + path.file;

        int texWidth = 0, texHeight = 0, texComponents = 0;
        auto imageBuffer = stbi_load(file.c_str(), &texWidth, &texHeight, &texComponents, 0);
        if (imageBuffer) {
            if (texture.val) {
                core.renderer->Release(*texture.val);
            }

            auto format = texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB;
            texture.val = createTextureFromData(core.renderer.get(), format,
                    imageBuffer, texWidth, texHeight);

            stbi_image_free(imageBuffer);
        } else {
            std::cerr << "failed to load image from file: " << file << std::endl;
        };
    }

    void importTexture(flecs::world &ecs) {
        ecs.system<>("regTexture", "Texture").kind(flecs::OnAdd).each(regTexture);
        ecs.system<>("unregTexture", "Texture").kind(flecs::OnRemove).each(unregTexture);
        ecs.system< const RegTo, TextureRes, const Path>("updateTexture", "Texture").
                kind(flecs::OnSet).each(updateTexture);
    }
}