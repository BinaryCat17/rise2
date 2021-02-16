#include <entt/entt.hpp>
#include <iostream>

struct position {
    float x;
    float y;
};

struct velocity {
    float x;
    float y;
};

std::ostream &operator<<(std::ostream &os, const velocity &velocity) {
    os << "x: " << velocity.x << " y: " << velocity.y;
    return os;
}

void print(entt::registry & r, entt::entity e) {
    std::cout << r.get<velocity>(e) << std::endl;
}

int main() {
    entt::registry registry;
    registry.on_construct<velocity>().connect<&print>();
    registry.on_update<velocity>().connect<&print>();

    auto entity = registry.create();
    auto entity1 = registry.create();
    registry.emplace<position>(entity, 10.f, 2.f);
    registry.emplace<velocity>(entity, 10.f, 2.f);
    registry.emplace<velocity>(entity1, 5.f, 5.f);

    registry.view<velocity>().each([](auto entity, auto& vel) {
       std::cout << vel << std::endl;
    });

    registry.remove<position>(entity);
    registry.destroy(entity);
}