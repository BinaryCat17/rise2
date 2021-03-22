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

void onAdd(flecs::entity) {
    std::cout << "fdsf" << std::endl;
}


int main(int argc, char *argv[]) {
    flecs::world ecs(argc, argv);

    ecs.component<Position>();
    ecs.component<Velocity>();

    ecs.system<>(nullptr, "PARENT:Velocity").iter([](flecs::iter it) {
        flecs::column<const Velocity>(it, 1);
        for(auto row : it) {
            it.entity(row).get_parent<Velocity>().set<Velocity>({});
        }
    });

    auto e = ecs.entity("e").set<Velocity>({});

    auto e1 = ecs.entity("e1").set<Position>({1, 0});
    auto e2 = ecs.entity("e2").set<Position>({0, 1});

    e1.add_childof(e);
    e2.add_childof(e);

    ecs.set_target_fps(1);
    while (ecs.progress()) {

    }
}