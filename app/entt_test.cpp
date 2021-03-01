#include <iostream>
#include "system.hpp"

using namespace rise;

struct MyVisitor : Visitor<MyVisitor> {
    using Visitor::visit;

    static void visit(entt::registry&, entt::entity, float v) {
        std::cout << v << " ";
    }

    template<typename T>
    static void visit(entt::registry&, entt::entity, NestedBegin<T>) {
        std::cout << "{ ";
    }

    template<typename T>
    static void visit(entt::registry&, entt::entity, NestedEnd<T>) {
        std::cout << "}" << std::endl;
    }
};

int main() {
    entt::registry registry;
    auto entity1 = registry.create();
    auto entity2 = registry.create();

    registry.emplace<Position>(entity1, glm::vec3(10.f, 2.f, 0.0f));
    registry.emplace<Scale>(entity1, glm::vec3(70.f, 2.f, 0.0f));
    registry.emplace<Position>(entity2, glm::vec3(5.f, 5.f, 0.0f));

    visit<MyVisitor>(registry, entity1);
    visit<MyVisitor>(registry, entity2);
}