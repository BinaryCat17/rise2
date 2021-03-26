#define STB_IMAGE_IMPLEMENTATION

#include "texture.hpp"
#include "stb_image.h"
#include "utils.hpp"

namespace rise::rendering {
    void regTexture(flecs::entity e) {
        if (!e.owns<Initialized>()) {
            if (!e.has<Path>()) e.set<Path>({});
            e.set<TextureId>({});
        }
    }

    void initTexture(flecs::entity e, TextureId &id) {
        std::tuple init{TextureState{}, std::vector<flecs::entity>{}};
        id.id = getApp(e)->manager.texture.states.push_back(std::move(init));
        e.add<Initialized>();
    }

    void unregTexture(flecs::entity e) {
        if (e.owns<Initialized>()) {
            getApp(e)->manager.texture.toRemove.push_back(*e.get<TextureId>());
            e.remove<TextureId>();
            e.remove<Initialized>();
        }
    }

    // при изменении пути до файла отложенно обновляем текстуру
    void updateTexture(flecs::entity, ApplicationRef ref, TextureId texture, Path const &path) {
        auto &root = ref.ref.entity().get<Path>()->file;
        auto &manager = ref.ref->id->manager;
        auto renderer = ref.ref->id->core.renderer.get();

        auto file = root + "/textures/" + path.file;

        int width = 0, height = 0, components = 0;
        auto image = stbi_load(file.c_str(), &width, &height, &components, 0);

        if (image) {
            TextureState state;
            auto format = components == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB;
            state.val = createTextureFromData(renderer, format, image, width, height);

            manager.texture.toInit.emplace_back(state, texture);

            stbi_image_free(image);
        } else {
            std::cerr << "failed to load image from file: " << file << std::endl;
        }
    }

    void importTexture(flecs::world &ecs) {
        ecs.system<>("regTexture", "Texture").kind(flecs::OnAdd).each(regTexture);
        ecs.system<>("unregTexture", "Texture").kind(flecs::OnRemove).each(unregTexture);
        ecs.system<TextureId>("initTexture", "OWNED:ApplicationRef, !Initialized").kind(flecs::OnSet).each(initTexture);
        ecs.system<const ApplicationRef, const TextureId, const Path>("updateTexture",
                "Texture, Initialized").kind(flecs::OnSet).each(updateTexture);
    }
}