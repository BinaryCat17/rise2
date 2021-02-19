#include "resources.hpp"
#include "shader.hpp"
#include "scene.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise::resources {
  using namespace rise;

    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            std::cerr << "error: " << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            std::cerr << "warning: " << message.GetText() << std::endl;
        }
    };

    std::unique_ptr<LLGL::RenderSystem> createRenderer() {
        static Debugger debugger;
        return LLGL::RenderSystem::Load("OpenGL", nullptr, &debugger);
    }

    LLGL::RenderContext *createContext(LLGL::RenderSystem *renderer,
            unsigned width, unsigned height) {
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = {width, height};
        contextDesc.videoMode.fullscreen = false;
        contextDesc.vsync.enabled = true;
        contextDesc.samples = 8;
        LLGL::RenderContext *context = renderer->CreateRenderContext(contextDesc);

        const auto &info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        return context;

    }

    Instance createInstance(unsigned width, unsigned height) {
        Instance instance = {};
        instance.renderer = createRenderer();
        instance.context = createContext(instance.renderer.get(), width, height);

        auto &window = LLGL::CastTo<LLGL::Window>(instance.context->GetSurface());
        window.SetTitle(L"Rise!");
        window.Show();
        instance.window = &window;

        return instance;
    }

    void reCalc(entt::registry &r, entt::entity e) {
        glm::mat4 mat(1);
        auto &pos = r.get<scene::Position>(e);
        mat = glm::translate(mat, pos);
        if (auto rotation = r.try_get<scene::Rotation>(e)) {
            mat = glm::rotate(mat, glm::radians(90.f), *rotation);
        }
        if (auto scale = r.try_get<scene::Scale>(e)) {
            mat = glm::scale(mat, *scale);
        }

        auto &instance = r.ctx<resources::Instance>();
        auto model = r.get<scene::ModelData>(e);
        mapUniformBuffer<glm::mat4>(instance.renderer.get(), model.uniformBuffer, [&mat](glm::mat4 *m) {
            *m = mat;
        });
    }

    void initRenderer(entt::registry &r, unsigned width, unsigned height, std::string const &root) {
        auto const &instance = r.set<Instance>(createInstance(width, height));

        r.on_construct<scene::Position>().connect<&reCalc>();
        r.on_update<scene::Position>().connect<&reCalc>();

        r.on_construct<scene::Rotation>().connect<&reCalc>();
        r.on_update<scene::Rotation>().connect<&reCalc>();

        r.on_construct<scene::Scale>().connect<&reCalc>();
        r.on_update<scene::Scale>().connect<&reCalc>();
    }
}
