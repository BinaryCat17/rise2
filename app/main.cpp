#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering.hpp"

int main(int argc, char *argv[]) {
    std::string root = "/home/gaben/projects/rise";
    unsigned width = 800;
    unsigned height = 600;

    auto renderer = rise::createRenderer();
    auto context = rise::createContext(renderer.get(), width, height);
    auto [vertexBuffer, format] = rise::createVertexBuffer(renderer.get());
    auto shader = rise::createShaderProgram(renderer.get(), root, format);
    auto [pipeline, layout] = rise::createPipeline(renderer.get(), shader);

    LLGL::CommandQueue *cmdQueue = renderer->GetCommandQueue();
    LLGL::CommandBuffer *cmdBuffer = renderer->CreateCommandBuffer();

    auto resources = rise::createResources(renderer.get(), layout);

    while (window.ProcessEvents()) {
        cmdBuffer->Begin();
        cmdBuffer->SetVertexBuffer(*vertexBuffer);
        cmdBuffer->BeginRenderPass(*window);
        cmdBuffer->SetViewport(context->GetResolution());
        cmdBuffer->Clear(LLGL::ClearFlags::Color);
        cmdBuffer->UpdateBuffer(*uniformBuffer, 0, &mvp, sizeof(glm::mat4));
        cmdBuffer->Draw(3, 0);
        cmdBuffer->EndRenderPass();
        cmdBuffer->End();
        cmdQueue->Submit(*cmdBuffer);
        context->Present();
    }
}
