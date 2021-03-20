#include <flecs.h>
#include <iostream>

struct Pos {
    int32_t x;
    int32_t y;
};

struct Vel {
    int32_t x;
    int32_t y;
};

void setS(flecs::entity e, Pos p) {
    std::cout << "set" << std::endl;
    e.set<Pos>({});
    e.set<Vel>({});
}

static int n = 0;
void printOnSet(flecs::entity e, Pos p, Vel v) {
    std::cout << ++n << " ------------------" << std::endl;
    std::cout << p.x << " " << p.y << std::endl;
    std::cout << v.x << " " << v.y << std::endl;
}

int main(int argc, char *argv[]) {
    flecs::world world(argc, argv);
    world.system<Pos, Vel>(nullptr).kind(flecs::OnSet).each(printOnSet);
    world.system<Pos>().each(setS);

    auto e = world.entity().set<Pos>({1, 2});
    e.set<Vel>({3, 4});

    world.set_target_fps(1);
    while(world.progress()) {};
}

