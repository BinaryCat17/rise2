#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <iostream>

namespace rise {
    template<typename... Types, typename... Ctx>
    void start(Ctx &&... ctx) {
        entt::registry r;
        (r.set<Ctx>(ctx), ...);
        (Types::init(r), ...);
        while ((Types::update(r) && ...)) {}
        (Types::destroy(r), ...);
    }

    struct Position {
        glm::vec3 val = {};
    };

    struct Rotation {
        glm::vec3 val = {};
    };

    struct Scale {
        glm::vec3 val = {};
    };

    struct DiffuseColor {
        glm::vec3 val = {};
    };

    struct Path {
        std::string val;
    };

    struct Mesh {};

    struct Texture {};

    enum class Shading {
        Diffuse,
    };

    struct Distance {
        float val;
    };

    struct Intensity {
        float val;
    };

    struct Drawable {};

    struct PointLight {};

    struct Group {
        std::vector<entt::entity> children;
    };

    struct SubEntities {
        std::vector<entt::entity> children;
    };

    template<typename>
    struct NestedBegin {};

    template<typename>
    struct NestedEnd {};

    struct RenderNode {};

    template<typename T>
    T *deepTryGet(entt::registry &r, entt::entity e) {
        if (auto v = r.try_get<T>(e)) {
            return v;
        } else {
            if (auto children = r.try_get<SubEntities>(e)) {
                for (auto c: children->children) {
                    if (auto res = deepTryGet<T>(r, c)) {
                        return res;
                    }
                }
                return nullptr;
            } else {
                return nullptr;
            }
        }
    }

    template<typename VT, typename T>
    auto doVisitType(entt::registry &r, entt::entity e, entt::type_info const &info) {
        if (info.hash() == entt::type_hash<T>().value()) {
            if constexpr(std::is_empty_v<T>) {
                return VT::visit(r, e, T{});
            } else {
                return VT::visit(r, e, r.get<T>(e));
            }
        }
    }

    template<typename VT, typename... Types>
    auto doVisit(entt::registry &r, entt::entity e) {
        return r.template visit(e, [&r, e](entt::type_info const &info) {
            (doVisitType<VT, Types>(r, e, info), ...);
        });
    }

    template<typename VT>
    auto visit(entt::registry &r, entt::entity e) {
        doVisit<VT,
                Position,
                Rotation,
                Scale,
                DiffuseColor,
                Group,
                SubEntities,
                Path,
                Mesh,
                Texture,
                Intensity,
                Distance,
                Shading,
                Drawable,
                PointLight,
                RenderNode
        >(r, e);
    }

    template<typename VT>
    struct Visitor {
        template<typename T>
        static auto visit(entt::registry &r, entt::entity e, T const &v)
        -> decltype((void) (v.val), void()) {
            VT::visit(r, e, NestedBegin<T>{});
            VT::visit(r, e, v.val);
            VT::visit(r, e, NestedEnd<T>{});
        }

        static auto visit(entt::registry &, entt::entity, ...) {}

        static void visit(entt::registry &r, entt::entity e, glm::vec2 v) {
            VT::visit(r, e, NestedBegin<glm::vec2>{});
            VT::visit(r, e, v.x);
            VT::visit(r, e, v.y);
            VT::visit(r, e, NestedEnd<glm::vec2>{});
        }

        static void visit(entt::registry &r, entt::entity e, glm::vec3 v) {
            VT::visit(r, e, NestedBegin<glm::vec3>{});
            VT::visit(r, e, v.x);
            VT::visit(r, e, v.y);
            VT::visit(r, e, v.z);
            VT::visit(r, e, NestedEnd<glm::vec3>{});
        }

        static void visit(entt::registry &r, entt::entity e, glm::vec4 v) {
            VT::visit(r, e, NestedBegin<glm::vec4>{});
            VT::visit(r, e, v.x);
            VT::visit(r, e, v.y);
            VT::visit(r, e, v.z);
            VT::visit(r, e, v.w);
            VT::visit(r, e, NestedEnd<glm::vec4>{});
        }

        static void visit(entt::registry &r, entt::entity e, Group const &v) {
            VT::visit(r, e, NestedBegin<Group>{});
            for (auto c : v.children) {
                rise::visit<VT>(r, c);
            }
            VT::visit(r, e, NestedEnd<Group>{});
        }

        static void visit(entt::registry &r, entt::entity e, SubEntities const &v) {
            VT::visit(r, e, NestedBegin<SubEntities>{});
            for (auto c : v.children) {
                rise::visit<VT>(r, c);
            }
            VT::visit(r, e, NestedEnd<SubEntities>{});
        }
    };

    template<typename>
    struct TypeInfo;

    template<>
    struct TypeInfo<glm::vec2> {
        static char const *name() {
            return "vec2";
        }
    };

    template<>
    struct TypeInfo<glm::vec3> {
        static char const *name() {
            return "vec3";
        }
    };

    template<>
    struct TypeInfo<glm::vec4> {
        static char const *name() {
            return "vec4";
        }
    };

    template<>
    struct TypeInfo<Position> {
        static char const *name() {
            return "Position";
        }
    };

    template<>
    struct TypeInfo<Rotation> {
        static char const *name() {
            return "Rotation";
        }
    };

    template<>
    struct TypeInfo<Scale> {
        static char const *name() {
            return "Scale";
        }
    };

    template<>
    struct TypeInfo<DiffuseColor> {
        static char const *name() {
            return "DiffuseColor";
        }
    };

    template<>
    struct TypeInfo<Group> {
        static char const *name() {
            return "Group";
        }
    };

    template<>
    struct TypeInfo<SubEntities> {
        static char const *name() {
            return "SubEntities";
        }
    };

    template<>
    struct TypeInfo<Path> {
        static char const *name() {
            return "Path";
        }
    };

    template<>
    struct TypeInfo<Mesh> {
        static char const *name() {
            return "Mesh";
        }
    };

    template<>
    struct TypeInfo<Texture> {
        static char const *name() {
            return "Texture";
        }
    };

    template<>
    struct TypeInfo<Drawable> {
        static char const *name() {
            return "Drawable";
        }
    };

    template<>
    struct TypeInfo<Distance> {
        static char const *name() {
            return "Distance";
        }
    };

    template<>
    struct TypeInfo<Intensity> {
        static char const *name() {
            return "Intensity";
        }
    };

    template<>
    struct TypeInfo<Shading> {
        static char const *name() {
            return "Shading";
        }
    };

    template<>
    struct TypeInfo<PointLight> {
        static char const *name() {
            return "PointLight";
        }
    };

    template<>
    struct TypeInfo<RenderNode> {
        static char const *name() {
            return "RenderNode";
        }
    };
}