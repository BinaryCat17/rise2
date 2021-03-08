#pragma once

#include <flecs.h>
#include <flecs_meta.h>

namespace rise {
    template<typename T>
    T const *getOrDefault(flecs::world &ecs) {
        static T defaultValue{};
        auto val = ecs.get<T>();
        if (val) {
            return val;
        } else {
            return &defaultValue;
        }
    }

    template<typename T>
    T &checkGet(flecs::world &ecs) {
        auto val = ecs.get_mut<T>();
        assert(val);
        return *val;
    }

    template<typename T>
    T &getOrSet(flecs::world &ecs) {
        auto val = ecs.get_mut<T>();
        if (val) {
            return *val;
        } else {
            ecs.set<T>({});
            return *ecs.get_mut<T>();
        }
    }

    template<typename T>
    T const *getOrDefault(flecs::entity e) {
        static T defaultValue{};
        auto val = e.get<T>();
        if (val) {
            return val;
        } else {
            return &defaultValue;
        }
    }

    template<typename T>
    T &checkGet(flecs::entity e) {
        auto val = e.get_mut<T>();
        assert(val);
        return *val;
    }

    template<typename T>
    T &getOrSet(flecs::entity e) {
        auto val = e.get_mut<T>();
        if (val) {
            return *val;
        } else {
            e.set<T>({});
            return e.get<T>();
        }
    }

    template<typename T>
    T *getFromIt(flecs::iter it, size_t i, T *arr) {
        if (it.is_owned(1)) {
            return &arr[i];
        } else {
            return arr;
        }
    }
}