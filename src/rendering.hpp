#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace rise {
    namespace impl {
        struct MeshRes {
            LLGL::Buffer *vertices = nullptr;
            LLGL::Buffer *indices = nullptr;
            unsigned numIndices = 0;
        };

        struct ModelRes {
            LLGL::Buffer *uniformBuffer;
            LLGL::ResourceHeap *heap;
        };

        struct Resources {
            std::vector<MeshRes> meshes;
        };

        struct GlobalShaderData {
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 projection;
        };

        struct ModelData {
            alignas(16) glm::mat4 transofrm;
        };
    }

    struct Mesh {
        size_t id;
    };

    struct Position : glm::vec3 {};

    struct Rotation : glm::vec3 {};

    enum class Shading {
        Diffuse,
    };

    enum class CameraMode {
        FullControl
    };

    struct Drawable {
        Shading shadingType;
    };

    struct Instance {
        std::unique_ptr<LLGL::RenderSystem> renderer;
        LLGL::RenderContext *context;
        LLGL::Window *window;
        LLGL::PipelineState *pipeline;
        LLGL::PipelineLayout *layout;
        LLGL::ShaderProgram *program;
        impl::Resources resources;
        LLGL::Buffer *globalShaderData;
        std::string root;
    };

    Instance makeInstance(std::string const &root, unsigned width, unsigned height);

    void init(entt::registry &r, Instance *instance);

    void setCameraView(entt::registry &r, glm::vec3 pos, glm::vec3 origin);

    Mesh loadMesh(entt::registry &r, std::string const &path);

    void setActiveCamera(entt::registry& r, entt::entity e, CameraMode mode);

    void renderLoop(entt::registry &r);
}