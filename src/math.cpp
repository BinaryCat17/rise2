#include "math.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rise {
	glm::mat4 cameraView(glm::mat4 model, unsigned width, unsigned height) {
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

		glm::mat4 view = glm::lookAt(
			glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		return projection * view * model;
	}
}
