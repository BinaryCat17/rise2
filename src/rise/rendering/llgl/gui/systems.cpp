#include "systems.hpp"
#include "../core/utils.hpp"
#include "pipeline.hpp"
#include <backends/imgui_impl_sdl.h>

namespace rise::rendering {
    void updateResources(flecs::entity, CoreState &core, GuiState &gui, GuiContext context) {
        ImGui::SetCurrentContext(context.context);
        ImGuiIO &io = ImGui::GetIO();
        ImDrawData *imDrawData = ImGui::GetDrawData();

        guiPipeline::Global parameters = {};
        parameters.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        parameters.translate.x = -1.0f - imDrawData->DisplayPos.x * parameters.scale.x;
        parameters.translate.y = -1.0f - imDrawData->DisplayPos.y * parameters.scale.y;

        updateUniformBuffer(core.renderer.get(), gui.uniform, parameters);

        if ((imDrawData->TotalVtxCount == 0) || (imDrawData->TotalIdxCount == 0)) {
            return;
        }

        std::vector<ImDrawVert> vertices;
        std::vector<ImDrawIdx> indices;

        for (int n = 0; n < imDrawData->CmdListsCount; n++) {
            const ImDrawList *cmd_list = imDrawData->CmdLists[n];
            vertices.insert(vertices.end(), cmd_list->VtxBuffer.Data,
                    cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size);

            indices.insert(indices.end(), cmd_list->IdxBuffer.Data,
                    cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size);
        }

        auto &mesh = gui.guiMesh;
        auto renderer = core.renderer;

        if (mesh.numVertices != imDrawData->TotalVtxCount) {
            if (mesh.vertices) {
                renderer->Release(*mesh.vertices);
            }
            mesh.numVertices = imDrawData->TotalVtxCount;
            mesh.vertices = createVertexBuffer(renderer.get(), gui.format, vertices);
        } else {
            renderer->WriteBuffer(*mesh.vertices, 0, vertices.data(),
                    vertices.size() * sizeof(ImDrawVert));
        }

        if (mesh.numIndices != imDrawData->TotalIdxCount) {
            if (mesh.indices) {
                renderer->Release(*mesh.indices);
            }
            mesh.numIndices = imDrawData->TotalIdxCount;
            mesh.indices = createIndexBuffer(renderer.get(), indices);
        } else {
            renderer->WriteBuffer(*mesh.indices, 0, indices.data(),
                    indices.size() * sizeof(ImDrawIdx));
        }
    }

    void prepareImgui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context) {
        ImGui::SetCurrentContext(context.context);

        int width, height;
        SDL_GetWindowSize(core.window, &width, &height);

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        ImGui_ImplSDL2_NewFrame(core.window);
        ImGui::NewFrame();
    }

    void processImGui(flecs::entity, GuiContext context) {
        ImGui::SetCurrentContext(context.context);
        ImGui::Render();
    }

    void renderGui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context,
            Extent2D size) {
        ImGui::SetCurrentContext(context.context);

        LLGL::Viewport viewport;
        viewport.width = static_cast<float>(size.width);
        viewport.height = -static_cast<float>(size.height);
        viewport.x = 0;
        viewport.y = static_cast<float>(size.height);

        auto cmdBuf = core.cmdBuf;

        cmdBuf->SetViewport(viewport);

        cmdBuf->SetPipelineState(*gui.pipeline);
        cmdBuf->SetResourceHeap(*gui.heap);

        ImDrawData *imDrawData = ImGui::GetDrawData();
        float fb_width = (imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
        float fb_height = (imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);
        ImVec2 clip_off = imDrawData->DisplayPos;
        ImVec2 clip_scale = imDrawData->FramebufferScale;
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0) {
            cmdBuf->SetVertexBuffer(*gui.guiMesh.vertices);
            cmdBuf->SetIndexBuffer(*gui.guiMesh.indices);

            for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imDrawData->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                    const ImDrawCmd *pCmd = &cmd_list->CmdBuffer[j];
                    ImVec4 clip_rect;
                    clip_rect.x = (pCmd->ClipRect.x - clip_off.x) * clip_scale.x;
                    clip_rect.y = (pCmd->ClipRect.y - clip_off.y) * clip_scale.y;
                    clip_rect.z = (pCmd->ClipRect.z - clip_off.x) * clip_scale.x;
                    clip_rect.w = (pCmd->ClipRect.w - clip_off.y) * clip_scale.y;

                    if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f &&
                            clip_rect.w >= 0.0f) {
                        LLGL::Scissor scissor;
                        scissor.x = std::max((int32_t) (clip_rect.x), 0);
                        scissor.y = std::max((int32_t) (clip_rect.y), 0);
                        scissor.width = static_cast<int32_t>(clip_rect.z - clip_rect.x);
                        scissor.height = static_cast<int32_t>(clip_rect.w - clip_rect.y);

                        cmdBuf->SetScissor(scissor);
                        cmdBuf->DrawIndexed(pCmd->ElemCount, indexOffset + pCmd->IdxOffset,
                                vertexOffset + static_cast<int32_t>(pCmd->VtxOffset));
                    }
                }
                vertexOffset += cmd_list->VtxBuffer.Size;
                indexOffset += cmd_list->IdxBuffer.Size;
            }
        }
    }

}