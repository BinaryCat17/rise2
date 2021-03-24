#define STB_IMAGE_IMPLEMENTATION
#include "resources.hpp"
#include "stb_image.h"
#include "utils.hpp"
#include <iostream>

namespace rise::rendering {
    void regTexture(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({});
        auto app = getApp(e);
        e.set<TextureId>({ app->manager.textures.push_back(std::tuple{TextureState{}})});
    }

    void unregTexture(flecs::entity e) {
        auto app = getApp(e);
        app->manager.texturesToRemove.push_back(*e.get<TextureId>());
        e.remove<TextureId>();
    }

    void updateTexture(flecs::entity, ApplicationRef ref, TextureId texture, Path const &path) {
        auto& root = ref.ref.entity().get<Path>()->file;
        auto& app = *ref.ref->id;

        auto file = root + "/textures/" + path.file;

        int width = 0, height = 0, components = 0;
        auto imageBuffer = stbi_load(file.c_str(), &width, &height, &components, 0);

        if (imageBuffer) {
            auto format = components == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB;

            TextureState state;
            state.val = createTextureFromData(app.core.renderer.get(), format, imageBuffer, width, height);

            stbi_image_free(imageBuffer);

            app.manager.texturesToRemove.emplace_back(texture);
            app.manager.texturesToInit.emplace_back(state, texture);

        } else {
            std::cerr << "failed to load image from file: " << file << std::endl;
        }
    }

    void prepareDeferTexture(flecs::entity, ApplicationId app) {
        auto& manager = app.id->manager;
        for(auto texture : manager.texturesToRemove) {
            auto& models = std::get<eTextureModels>(manager.textures.at(texture.id)).get();
            for(auto model : models) {
                manager.updatedModels.push_back(model);
            }
        }
    }

    void processDeferTexture(flecs::entity, ApplicationId app) {
        auto& manager = app.id->manager;
        for(auto texture : manager.texturesToRemove) {
            if(contains(manager.textures, texture.id)) {
                auto& state = std::get<eTextureState>(manager.textures.at(texture.id)).get();
                app.id->core.renderer->Release(*state.val);
            }
        }

        for(auto texture : manager.texturesToInit) {
            auto& state = std::get<eTextureState>(manager.textures.at(texture.second.id)).get();
            state = texture.first;

        }
    }

    void importTexture(flecs::world &ecs) {
        ecs.system<>("regTexture", "Texture").kind(flecs::OnAdd).each(regTexture);
        ecs.system<>("unregTexture", "Texture").kind(flecs::OnRemove).each(unregTexture);
        ecs.system<const ApplicationRef, const TextureId, const Path>("updateTexture", "Texture").
                kind(flecs::OnSet).each(updateTexture);
    }
}