#pragma once

#include <LLGL/LLGL.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

struct SDL_Window;

namespace rise {
    const size_t maxLightCount = 32;

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
            std::vector<LLGL::Texture *> textures;
        };

        struct LightId {
            size_t id;
        };

        struct PointLight {
            alignas(16) glm::vec3 position = {};
            alignas(16) glm::vec3 diffuse = {};
            alignas(4) float constant = 0;
            alignas(4) float linear = 0;
            alignas(4) float quadratic = 0;
        };

        struct GlobalShaderData {
            alignas(16) glm::mat4 view = {};
            alignas(16) glm::mat4 projection = {};
            alignas(16) PointLight pointLights[maxLightCount] = {};
        };

        struct ModelData {
            alignas(16) glm::mat4 transform = {};
        };

        struct CameraInfo {
            entt::entity camera{entt::null};
            bool relative = false;
        };
    }

    struct Mesh {
        size_t id;
    };

    struct Texture {
        size_t id;
    };

    struct Position : glm::vec3 {};

    struct Rotation : glm::vec3 {};

    struct Scale : glm::vec3 {};

    struct DiffuseColor : glm::vec3 {};

    enum class Shading {
        Diffuse,
    };

    enum class CameraMode {
        FullControl
    };

    struct Drawable {
        Shading shadingType;
    };

    struct PointLight {
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
    };

    struct Instance {
        std::unique_ptr<LLGL::RenderSystem> renderer;
        LLGL::RenderContext *context;
        SDL_Window *window;
        LLGL::PipelineState *pipeline;
        LLGL::PipelineLayout *layout;
        LLGL::ShaderProgram *program;
        impl::Resources resources;
        LLGL::Buffer *globalShaderData;
        std::string root;
        impl::CameraInfo camera;
        LLGL::Sampler *sampler;
        LLGL::Texture *defaultTexture;
        size_t lightCount;
    };

    Instance makeInstance(std::string const &root, unsigned width, unsigned height);

    void init(entt::registry &r, Instance *instance);

    void setCameraView(entt::registry &r, glm::vec3 pos, glm::vec3 origin);

    Mesh loadMesh(entt::registry &r, std::string const &path);

    Texture loadTexture(entt::registry &r, std::string const &path);

    void setActiveCamera(entt::registry &r, entt::entity e, CameraMode mode);

    void renderLoop(entt::registry &r);
}