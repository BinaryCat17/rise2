#define STB_IMAGE_IMPLEMENTATION

#include "texture.hpp"
#include "stb_image.h"
#include "utils.hpp"

namespace rise::rendering {
    void regTexture(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({});
        e.set<TextureId>({});
    }

    void initTexture(flecs::entity e, ApplicationRef app, TextureId &id) {
        if (!e.has_trait<Initialized, TextureId>()) {
            std::tuple init{TextureState{}, std::set<flecs::entity_t>{}};
            id.id = app.ref->id->manager.texture.states.push_back(std::move(init));
            e.add_trait<Initialized, TextureId>();
            e.patch<Path>([](auto) {});
        }
    }

    void unregTexture(flecs::entity e) {
        if (e.has_trait<Initialized, TextureId>()) {
            e.remove<TextureId>();
            e.remove_trait<Initialized, TextureId>();
        }
    }

    void removeTexture(flecs::entity, ApplicationRef app, TextureId id) {
        app.ref->id->manager.texture.toRemove.push_back(id);
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

    void regTextureToModel(flecs::entity e, ApplicationRef app, flecs::entity te) {
        auto &manager = app.ref->id->manager;
        auto tId = te.get<TextureId>();
        if(tId) {
            auto &models = std::get<eTextureModels>(manager.texture.states.at(tId->id)).get();

            auto& prev = *e.get_trait_mut<Previous, TextureId>();
            if (prev.e != flecs::entity(0)) {
                models.erase(prev.e.id());
            }

            models.insert(e.id());
            prev.e = e;

        }
    }

    void unregTextureFromModel(flecs::entity e, ApplicationRef app, flecs::entity te) {
        auto &manager = app.ref->id->manager;
        auto tId = te.get<TextureId>();
        if(tId) {
            auto &models = std::get<eTextureModels>(manager.texture.states.at(tId->id)).get();
            models.erase(e.id());
        }
    }

    void regDifTextureToModel(flecs::entity e, ApplicationRef app, AlbedoTexture t) {
        regTextureToModel(e, app, t.e);
    }

    void unregDifTextureFromModel(flecs::entity e, ApplicationRef app, AlbedoTexture t) {
        unregTextureFromModel(e, app, t.e);
    }

    void regMetTextureToModel(flecs::entity e, ApplicationRef app, MetallicTexture t) {
        regTextureToModel(e, app, t.e);
    }

    void unregMetTextureFromModel(flecs::entity e, ApplicationRef app, MetallicTexture t) {
        unregTextureFromModel(e, app, t.e);
    }

    void regRoughTextureToModel(flecs::entity e, ApplicationRef app, RoughnessTexture t) {
        regTextureToModel(e, app, t.e);
    }

    void unregRoughTextureFromModel(flecs::entity e, ApplicationRef app, RoughnessTexture t) {
        unregTextureFromModel(e, app, t.e);
    }

    void importTexture(flecs::world &ecs) {
        ecs.system<>("regTexture", "Texture").kind(flecs::OnAdd).each(regTexture);
        ecs.system<>("unregTexture", "Texture").kind(flecs::OnRemove).each(unregTexture);
        ecs.system<const ApplicationRef, TextureId>("initTexture",
                "!TRAIT | Initialized > TextureId").kind(
                flecs::OnSet).each(initTexture);
        ecs.system<const ApplicationRef, const TextureId>("removeTexture").
                kind(EcsUnSet).each(removeTexture);
        ecs.system<const ApplicationRef, const AlbedoTexture>("regDifTextureToModel", "ModelId").
                kind(flecs::OnSet).each(regDifTextureToModel);
        ecs.system<const ApplicationRef, const AlbedoTexture>("unregDifTextureToModel", "ModelId").
                kind(EcsUnSet).each(unregDifTextureFromModel);
        ecs.system<const ApplicationRef, const MetallicTexture>("regMetTextureToModel", "ModelId").
                kind(flecs::OnSet).each(regMetTextureToModel);
        ecs.system<const ApplicationRef, const MetallicTexture>("unregMetTextureToModel", "ModelId").
                kind(EcsUnSet).each(unregMetTextureFromModel);
        ecs.system<const ApplicationRef, const RoughnessTexture>("regRoughTextureToModel", "ModelId").
                kind(flecs::OnSet).each(regRoughTextureToModel);
        ecs.system<const ApplicationRef, const RoughnessTexture>("unregRoughTextureToModel", "ModelId").
                kind(EcsUnSet).each(unregRoughTextureFromModel);
        ecs.system<const ApplicationRef, const TextureId, const Path>("updateTexture",
                "Texture, TRAIT | Initialized > TextureId").kind(flecs::OnSet).each(updateTexture);
    }
}