#include "sceneSystem.hpp"
#include "components/rendering/module.hpp"
#include "systems/rendering/resources/viewport.hpp"
#include "systems/rendering/resources/mesh.hpp"
#include "systems/rendering/resources/model.hpp"

using namespace rise::components::rendering;

namespace rise::systems::rendering {
	template<typename FnT>
	void forViewports(flecs::entity e, FnT&& f) {
		for (auto viewports : e.children()) {
			for (auto i : viewports) {
				auto viewportEntity = viewports.entity(i);
                auto viewport = viewportEntity.get_mut<ViewportResource>();
                auto pos = viewportEntity.get<Position2D>();
                auto extent = viewportEntity.get<Extent2D>();
				if (viewport && pos && extent) {
					f(viewportEntity, *viewport, *pos, *extent);
				}
			}
		}
	}

	template<typename FnT>
	void forModels(flecs::entity e, FnT&& f) {
		for (auto models : e.children()) {
			for (auto j : models) {
				auto modelEntity = models.entity(j);
                auto model = modelEntity.get_mut<ModelResource>();
                auto mesh = modelEntity.get_mut<MeshResource>();
				if (mesh && model) {
					f(modelEntity, *model, *mesh);
				}
			}
		}
	}

	void renderSceneSystem(flecs::entity e, ApplicationResource& app) {
	    app.state.cmdBuf->SetPipelineState(*app.scene.pipeline.pipeline);

		forViewports(e, [ &app](flecs::entity ve, ViewportResource& viewportRes, Position2D pos,
		        Extent2D extent) {
			LLGL::Viewport viewport{ pos.x, pos.y, extent.width, extent.height};

			auto resolution = app.state.context->GetResolution();
			assert(viewport.x + viewport.width <= resolution.width ||
				viewport.y + viewport.height <= resolution.height);

			app.state.cmdBuf->SetViewport(resolution);

			forModels(ve, [&app](flecs::entity, ModelResource& model, MeshResource& mesh) {
			    auto cmd = app.state.cmdBuf;
                cmd->SetResourceHeap(*model.heap);
                cmd->SetVertexBuffer(*mesh.vertices);
                cmd->SetIndexBuffer(*mesh.indices);
                cmd->DrawIndexed(mesh.numIndices, 0);
            });
        });
    }
}
