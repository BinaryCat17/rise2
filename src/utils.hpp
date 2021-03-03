#pragma once
#include <flecs.h>

namespace rise {
	template<typename T>
	T* getOrDefault(flecs::world &ecs) {
		static T defaultValue{};
		auto val = ecs.get<T>();
		if (val) {
			return val;
		}
		else {
			return &defaultValue;
		}
	}

	template<typename T>
	T& checkGet(flecs::world& ecs) {
		auto val = ecs.get<T>;
		assert(val);
		return *val;
	}
}