#include <iostream>
#include <flecs.h>
#include <rise/util/soa.hpp>

struct Position {
    float x;
    float y;
};

struct SomeRef {
    flecs::entity e;
};

int main() {
    flecs::world ecs;
    auto e = ecs.entity().set<Position>({5, 5});

    auto d =
}