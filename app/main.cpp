#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering.hpp"

int main(int argc, char *argv[]) {
    try {
        std::string root = "/home/gaben/projects/rise";
        unsigned width = 800;
        unsigned height = 600;

        auto renderer = rise::createRenderer();
        auto context = rise::createContext(renderer.get(), width, height);
        auto resources = rise::createShaderResources(renderer.get());
        auto pipeline = rise::createPipeline(renderer.get(), root, resources);

        rise::Camera camera = {
                width,
                height,
                {3, 4, 4}, // pos
                {0, 0, 0} // origin
        };

        rise::updateUniformData(renderer.get(), resources.camera, camera);

        renderLoop(renderer.get(), context, [&](LLGL::CommandBuffer *cmd) {
            rise::bindPipeline(cmd, pipeline);
            rise::bindResources(cmd, resources);
            rise::drawVertices(cmd, resources.vertex);
        });
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}
