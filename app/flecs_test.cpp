#include <iostream>
#include <flecs.h>

/* Ordinary position & velocity components */
struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct RenderTo {
    flecs::entity e;
};

struct LightTag {};

void onAdd(flecs::entity) {
    std::cout << "fdsf" << std::endl;
}


int main(int argc, char *argv[]) {
    flecs::world ecs(argc, argv);

    ecs.component<RenderTo>("RenderTo");
    ecs.component<LightTag>("LightTag");
    ecs.component<Position>();
    ecs.component<LightTag>();
    ecs.component<Velocity>();

    ecs.system<>(nullptr, "TRAIT | RenderTo > LightTag").kind(flecs::OnSet).each(onAdd);

    ecs.system<>(nullptr, "TRAIT | RenderTo")
            .iter([](flecs::iter it) {
                for(auto row : it) {
                    auto trait = it.column<RenderTo>(1);

                    auto e = it.entity(row);
                    auto t = e.get_trait<RenderTo, LightTag>();
                    std::cout << t->e.name() << std::endl;


                }
            });

    auto e1 = ecs.entity("e1").set<Position>({1, 0});
    auto e2 = ecs.entity("e2").set<Position>({0, 1});

    auto e = ecs.entity("e");

    e.set_trait<RenderTo, LightTag>({e1});

    std::cout << e.type().str() << std::endl;

    ecs.set_target_fps(1);
    while (ecs.progress()) {

    }
}