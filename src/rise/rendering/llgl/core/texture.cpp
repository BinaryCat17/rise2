#define STB_IMAGE_IMPLEMENTATION

#include "texture.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "rise/rendering/module.hpp"
#include "stb_image.h"
#include <iostream>

namespace rise::rendering {
    void regTexture(flecs::entity e) {
        if (e.owns<Texture>()) {
            if (!e.has<Path>()) e.set<Path>({});
            e.set<TextureRes>({});
        }
    }

    void unregTexture(flecs::entity e) {
        if (e.owns<Texture>()) {
            e.remove<TextureRes>();
        }
    }

    void removeTexture(flecs::entity, CoreState &core, TextureRes &texture) {
        if (texture.val) {
            core.renderer->Release(*texture.val);
        }
    }

    void updateTexture(flecs::entity e, CoreState &core, TextureRes &texture, Path const &path) {
        auto file = core.root + "/textures/" + path.file;

        int texWidth = 0, texHeight = 0, texComponents = 0;
        auto imageBuffer = stbi_load(file.c_str(), &texWidth, &texHeight, &texComponents, 0);
        if (imageBuffer) {
            removeTexture(e, core, texture);

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
        ecs.system<CoreState, TextureRes, Path>("updateTexture", "OWNED:TextureRes").
                kind(flecs::OnSet).each(updateTexture);
        ecs.system<CoreState, TextureRes>("removeTexture", "OWNED:TextureRes").
                kind(EcsUnSet).each(removeTexture);
    }
}