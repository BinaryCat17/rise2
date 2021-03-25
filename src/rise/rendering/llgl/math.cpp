#include "math.hpp"

namespace rise {
    glm::vec3 calcCameraOrigin(glm::vec3 position, glm::vec3 rotation) {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;
        direction.y = std::sin(glm::radians(rotation.z)) * 3;
        direction.z = std::sin(glm::radians(rotation.x)) * std::cos(glm::radians(rotation.z)) * 3;

        return position + direction;
    }
}
