#pragma once
#include <glm/glm.hpp>

namespace rise {
    struct CameraData {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    struct Camera {
        unsigned width = 0;
        unsigned height = 0;
        glm::vec3 position = {};
        glm::vec3 origin = {};
    };

    void updateCameraView(CameraData* cameraData, glm::vec3 position, glm::vec3 origin);

    void updateCameraProjection(CameraData* cameraData, unsigned width, unsigned height);
}