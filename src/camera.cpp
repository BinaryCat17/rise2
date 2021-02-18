#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
    void updateCameraView(CameraData *cameraData, glm::vec3 position, glm::vec3 origin) {
        cameraData->view = glm::lookAt(position, origin, glm::vec3(0, 1, 0));
    }

    void updateCameraProjection(CameraData *cameraData, unsigned width, unsigned height) {
        cameraData->projection = glm::perspective(glm::radians(45.0f),
                static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
    }
}