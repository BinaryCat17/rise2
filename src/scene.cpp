#include "scene.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
    void makeModel(entt::registry &r, entt::entity entity, size_t meshId, Position position) {
        auto &resources = r.ctx<SceneResources>();
        if (meshId >= resources.meshes.size()) {
            throw std::runtime_error("mesh doesn't exist");
        }

        r.emplace<Position>(entity, position);
        r.emplace<SceneResources::MeshId>(entity, meshId);
        glm::mat4 transformMatrix = glm::translate(glm::mat4(1), position);

        auto renderer = r.ctx<LLGL::RenderSystem *>();
        auto buffer = createUniformBuffer(renderer, transformMatrix);
        resources.uniformBuffers.push_back(buffer);
        r.emplace<SceneResources::UniformId>(entity,
                resources.uniformBuffers.size() - 1,
                TransformBinding);
    }

    void reCalc(entt::registry& r, entt::entity e) {
        glm::mat4 mat(1);
        auto& pos = r.get<Position>(e);
        mat = glm::translate(mat, pos);
        if(auto rotation = r.try_get<Rotation>(e)) {
            mat = glm::rotate(mat, glm::radians(90.f), *rotation);
        }
        if(auto scale = r.try_get<Scale>(e)) {
            mat = glm::scale(mat, *scale);
        }

        auto &resources = r.ctx<SceneResources>();
        auto buf = resources.uniformBuffers[r.get<SceneResources::UniformId>(e).index];
        auto renderer = r.ctx<LLGL::RenderSystem *>();
        mapUniformBuffer<glm::mat4>(renderer, buf, [&mat](glm::mat4* m){
            *m = mat;
        });
    }

    void initScene(entt::registry &r) {
        r.set<SceneResources>();
        r.on_construct<Position>().connect<&reCalc>();
        r.on_update<Position>().connect<&reCalc>();

        r.on_construct<Rotation>().connect<&reCalc>();
        r.on_update<Rotation>().connect<&reCalc>();

        r.on_construct<Scale>().connect<&reCalc>();
        r.on_update<Scale>().connect<&reCalc>();
    }
}