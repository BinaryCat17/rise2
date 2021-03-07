#include <rise/rendering/module.hpp>
#include <flecs_dash.h>
#include <flecs_systems_civetweb.h>
#include <rise/os/flecs_os.hpp>

using namespace rise;

int main() {
	stdcpp_set_os_api();

	flecs::world ecs;
	//ecs.import<flecs::dash>();
	//ecs.import<flecs::systems::civetweb>();
	//ecs.import<flecs::components::meta>();
	ecs.entity().set<flecs::dash::Server>({ 9090 });
	ecs.import<RenderModule>();

	auto cube = ecs.entity("Cube").set<WorldPosition>({ glm::vec3(0, 0, 0) });

	auto camera = ecs.entity("Camera")
		.set<WorldPosition>({ glm::vec3(0, 1, 0) })
		.set<DiffuseColor>({ glm::vec3(1, 1, 1) })
		.set<Intensity>({ 1.f })
		.set<Viewport>({ {0, 0}, {800, 600} });

	RenderModule::renderTo(ecs, cube, camera);

	ecs.set_target_fps(60);
	while (ecs.progress()) {

	}
}
