#include "sceneSystem.hpp"
#include "components/rendering/module.hpp"
#include "systems/rendering/resources/viewport.hpp"
#include "systems/rendering/resources/application.hpp"

using namespace rise::components::rendering;

namespace rise::systems::rendering {
    template<typename FnT>
    void forViewports(flecs::entity e, FnT &&f) {
        for (auto viewports : e.children()) {
            for (auto i : viewports) {
                auto viewportEntity = viewports.entity(i);
                if (auto viewport = viewportEntity.get<ViewportResource>()) {
                    f(viewportEntity, *viewport);
                }
            }
        }
    }

    template<typename FnT>
    void forModels(flecs::entity e, FnT &&f) {
        for (auto models : e.children()) {
            for (auto j : models) {
                auto modelEntity = models.entity(j);
                if (auto model = modelEntity.get<ModelResource>()) {
                    f(modelEntity, *model);
                }
            }
        }
    }

    void renderSceneSystem(flecs::entity e, Application &app) {


        auto viewportEntity = it.entity(0).get_parent<ViewportResource>();
        auto pos = checkGet<Position2D>(viewportEntity);
        auto extent = checkGet<Extent2D>(viewportEntity);

        auto &state = checkGet<ApplicationResource>(
                viewportEntity.get_parent<ApplicationResource>()).state;

        LLGL::Viewport viewport{pos.x, pos.y, extent.width, extent.height};

        auto resolution = application.context->GetResolution();
        assert(viewport.x + viewport.width <= resolution.width ||
                viewport.y + viewport.height <= resolution.height);
        instance.cmdBuf->SetViewport(resolution);

        for (auto i : it) {

        }

    }
}
