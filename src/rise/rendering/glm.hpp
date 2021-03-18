#pragma once
#include "module.hpp"
#include <glm/glm.hpp>

namespace rise::rendering {
    inline glm::vec2 toGlm(Extent2D val) {
        return {val.width, val.height};
    }

    inline glm::vec2 toGlm(Position2D val) {
        return {val.x, val.y};
    }

    inline glm::vec2 toGlm(Rotation2D val) {
        return {val.x, val.y};
    }

    inline glm::vec2 toGlm(Scale2D val) {
        return {val.x, val.y};
    }

    inline glm::vec3 toGlm(Extent3D val) {
        return {val.width, val.height, val.depth};
    }

    inline glm::vec3 toGlm(Position3D val) {
        return {val.x, val.y, val.z};
    }

    inline glm::vec3 toGlm(Rotation3D val) {
        return {val.x, val.y, val.z};
    }

    inline glm::vec3 toGlm(DiffuseColor val) {
        return {val.r, val.g, val.b};
    }

    inline glm::vec3 toGlm(Scale3D val) {
        return {val.x, val.y, val.z};
    }

    inline Extent2D fromGlmExtent2D(glm::vec2 val) {
        return {val.x, val.y};
    }

    inline Position2D fromGlmPosition2D(glm::vec2 val) {
        return {val.x, val.y};
    }

    inline Rotation2D fromGlmRotation2D(glm::vec2 val) {
        return {val.x, val.y};
    }

    inline Scale2D fromGlmScale2D(glm::vec2 val) {
        return {val.x, val.y};
    }

    inline Extent3D fromGlmExtent3D(glm::vec3 val) {
        return {val.x, val.y, val.z};
    }

    inline Position3D fromGlmPosition3D(glm::vec3 val) {
        return {val.x, val.y, val.z};
    }

    inline Rotation3D fromGlmRotation3D(glm::vec3 val) {
        return {val.x, val.y, val.z};
    }

    inline Scale3D fromGlmScale3D(glm::vec3 val) {
        return {val.x, val.y, val.z};
    }

    inline DiffuseColor fromGlmColorl(glm::vec3 val) {
        return {val.x, val.y, val.z};
    }
}