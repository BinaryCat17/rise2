#include "material.hpp"
#include "systems/rendering/util/resources.hpp"
#include "systems/rendering/pipeline/scene.hpp"
#include "application.hpp"

namespace rise::systems::rendering {
    void updateMaterials(flecs::iter it, MaterialResource *materials, DiffuseColor const* colors) {
        auto& state = it.column<ApplicationResource>(3)->state;
        for(auto i : it) {
            auto material = getFromIt(it, i, materials);
            auto color = getFromIt(it, i, colors);

            scenePipeline::PerMaterial data;
            data.diffuseColor = glm::vec4(color->r, color->g, color->b, 1);
            updateUniformBuffer(state.renderer.get(), material->uniform, data);
        }
    }
}
