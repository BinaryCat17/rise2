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


struct Module {
    struct F { };

    Module(flecs::world& ecs) {
        ecs.module<Module>("Module");

        auto e = ecs.entity().add<Module::F>();
        std::cout << "1 " << e.type().str() << std::endl;
    }
};

struct Module2 {
    Module2(flecs::world& ecs) {
        ecs.import<Module>();
        ecs.module<Module2>("M2");
        auto e = ecs.entity().add<Module::F>();
        std::cout << e.type().str() << std::endl;

        ecs.system<>("df", "M.Relative").each([](flecs::entity){});
    }
};

int main(int argc, char *argv[]) {
    flecs::world world(argc, argv);
    world.import<Module>();

    auto e = world.entity().add<Module::F>();
    std::cout << "1 " << e.type().str() << std::endl;

    world.progress();
}

