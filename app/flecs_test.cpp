#include <flecs.h>
#include <flecs_meta.h>
#include <iostream>
#include "../src/rise/components/rendering.hpp"

//
//ECS_STRUCT(Position, {
//    float x;
//    float y;
//});
//
//ECS_STRUCT(WorldPosition, {
//    float x;
//    float y;
//});
//
//ECS_STRUCT(Velocity, {
//    float x;
//    float y;
//});
//
//void Move(flecs::iter it, WorldPosition *p, Velocity const *v) {
//    std::cout << "Move called" << std::endl;
//    for (auto i : it) {
//        p[i] = {p[i].x + v[i].x, p[i].y + v[i].y};
//    }
//}
//
//struct OpHeader {};
//
//struct OpPush {};
//
//struct OpPop {};
//
//template<typename F>
//void visitPrimitive(ecs_primitive_kind_t type, void *data, F &&f) {
//
//    auto bytes = static_cast<uint8_t *>(data);
//    switch (type) {
//        case EcsBool:
//            f(reinterpret_cast<bool *>(bytes));
//            break;
//        case EcsChar:
//            f(reinterpret_cast<char *>(bytes));
//            break;
//        case EcsByte:
//            f(reinterpret_cast<flecs::byte *>(bytes));
//            break;
//        case EcsU8:
//            f(reinterpret_cast<uint8_t *>(bytes));
//            break;
//        case EcsU16:
//            f(reinterpret_cast<uint16_t *>(bytes));
//            break;
//        case EcsU32:
//            f(reinterpret_cast<uint32_t *>(bytes));
//            break;
//        case EcsU64:
//            f(reinterpret_cast<uint64_t *>(bytes));
//            break;
//        case EcsI8:
//            f(reinterpret_cast<int8_t *>(bytes));
//            break;
//        case EcsI16:
//            f(reinterpret_cast<int16_t *>(bytes));
//            break;
//        case EcsI32:
//            f(reinterpret_cast<int32_t *>(bytes));
//            break;
//        case EcsI64:
//            f(reinterpret_cast<int64_t *>(bytes));
//            break;
//        case EcsF32:
//            f(reinterpret_cast<float *>(bytes));
//            break;
//        case EcsF64:
//            f(reinterpret_cast<double *>(bytes));
//            break;
//        case EcsUPtr: // what is it?
//            assert(0);
//            break;
//        case EcsIPtr: // what is it?
//            assert(0);
//            break;
//        case EcsString:
//            f(reinterpret_cast<flecs::string>(bytes));
//            break;
//        case EcsEntity:
//            f(reinterpret_cast<flecs::entity_t *>(bytes));
//            break;
//    }
//}
//
//template<typename F>
//void visitStruct(EcsMetaTypeSerializer serializer, void *data, F &&f) {
//    auto bytes = static_cast<uint8_t *>(data);
//    auto *ops = (ecs_type_op_t *) ecs_vector_first(serializer.ops, ecs_type_op_t);
//    size_t count = ecs_vector_count(serializer.ops);
//
//    static OpHeader pop;
//    static OpHeader push;
//    static OpHeader header;
//
//    for (size_t i = 0; i != count; ++i) {
//        ecs_type_op_t *op = &ops[i];
//
//        switch (op->kind) {
//            case EcsOpHeader:
//                //f(&header);
//                break;
//            case EcsOpPush:
//                //f(&push);
//                break;
//            case EcsOpPop:
//                //f(&pop);
//                break;
//            case EcsOpPrimitive:
//                visitPrimitive(op->is.primitive, bytes + op->offset, f);
//                break;
//            default:
//                assert(0 && "not implemented");
//
//        }
//    }
//}
//
//int main() {
//    flecs::world ecs;
//    flecs::import<flecs::components::meta>(ecs);
//
//    flecs::meta<WorldPosition>(ecs);
//    flecs::meta<Position>(ecs);
//    flecs::meta<Velocity>(ecs);
//
//    auto v = ecs.entity().set<WorldPosition>({4, -5});
//    ecs.entity().add_instanceof(v).set<Velocity>({-4, 3});
//    ecs.entity().add_instanceof(v).set<Velocity>({-2, 1});
//    v.set<Position>({10, 3});
//
//    WorldPosition wp{1, 3};
//    std::cout << flecs::pretty_print(ecs, wp) << std::endl;
//
//    for (auto type : v.type().vector()) {
//        auto e = ecs.entity(type);
//        auto meta = e.get<EcsMetaType>();
//        auto serializer = e.get<EcsMetaTypeSerializer>();
//
//        std::cout << e.name() << meta->descriptor;
//        switch (meta->kind) {
//            case EcsStructType:
//                visitStruct(*serializer, v.get_mut(type),
//                        [](auto *v) {
//                            std::cout << *v << std::endl;
//                        });
//                break;
//            default:
//                assert(0 && "not implemented");
//        }
//    }
//}

ECS_STRUCT(Point, {
    int32_t x;
    int32_t y;
});

ECS_STRUCT(Line, {
    Point start;
    Point stop;
});

int main(int argc, char *argv[]) {
    flecs::world world(argc, argv);

    /* Import meta module */
    flecs::import<flecs::components::meta>(world);

    /* Insert the meta definitions for Position. This will also register the
     * Position type as a component */
    flecs::meta<Point>(world);
    flecs::meta<Line>(world);

    /* Create an instance of the Position type */
    Line l = {{10, 20},
              {30, 40}};

    /* Pretty print the value */
    std::cout << flecs::pretty_print(world, l) << std::endl;
}

