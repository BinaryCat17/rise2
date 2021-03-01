#include <flecs.h>
#include <iostream>

struct Position {
    float x;
    float y;
};

struct WorldPosition {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

void Move(flecs::entity e, Position const p, Velocity const &v) {
    e.set<Position>({p.x + v.x, p.y + v.y});
}

void OnWorldPosUpdate(flecs::entity e, WorldPosition const &p) {
    std::cout << "New " << e.name() << " position is: " << p.x << " " << p.y << std::endl;
    for (auto child : e.children()) {
        for (size_t i = 0; i != child.count(); ++i) {
            auto ch = child.entity(i);
            auto localPosition = ch.get<Position>();
            ch.set<WorldPosition>({localPosition->x + p.x, localPosition->y + p.y});
        }
    }
}

void OnLocalPosUpdate(flecs::entity e, Position const &p) {
    if(auto wpe = e.get_parent<WorldPosition>()) {
        auto wp = wpe.get<WorldPosition>();
        e.set<WorldPosition>({p.x + wp->x, p.y + wp->y});
    } else {
        e.set<WorldPosition>({p.x, p.y});
    }
}

int main() {
    flecs::world ecs;
    ecs.component<WorldPosition>();

    ecs.system<Position, Velocity>().each(Move);
    ecs.system<WorldPosition const>().kind(flecs::OnSet).each(OnWorldPosUpdate);
    ecs.system<Position const>().kind(flecs::OnSet).each(OnLocalPosUpdate);

    auto Root = ecs.entity("Root")
            .add<WorldPosition>()
            .set<Position>({0, 0})
            .set<Velocity>({1, 2});

    ecs.entity("Root2")
            .add<WorldPosition>()
            .set<Position>({0, 0});

    auto Child1 = ecs.entity("Child1")
            .add_childof(Root)
            .add<WorldPosition>()
            .set<Position>({100, 100});

    ecs.entity("GChild1")
            .add_childof(Child1)
            .add<WorldPosition>()
            .set<Position>({1000, 1000});

    auto Child2 = ecs.entity("Child2")
            .add_childof(Root)
            .add<WorldPosition>()
            .set<Position>({200, 200});

    ecs.entity("GChild2")
            .add_childof(Child2)
            .add<WorldPosition>()
            .set<Position>({2000, 2000});

    ecs.set_target_fps(1);
    while (ecs.progress()) {
        std::cout << "----" << std::endl;
    }
}