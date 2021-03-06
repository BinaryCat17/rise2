#include "renderGui.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include "resources.hpp"
#include "pipeline.hpp"

namespace rise {
    struct GuiState {
        PipelineData pipeline;
        LLGL::Buffer *globalData = nullptr;
        LLGL::Texture *fontTexture = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
        LLGL::Sampler *sampler = nullptr;
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned vertexCount = 0;
        unsigned indexCount = 0;
        entt::entity selectedEntity = entt::null;
        entt::delegate<void(entt::registry &r)> guiCallbacks{};
    };

    struct Parameters {
        alignas(8) glm::vec2 scale = glm::vec2(1.f, 1.f);
        alignas(8) glm::vec2 translate = glm::vec2(0.f, 0.f);
    };

    void initGui(entt::registry &r, LLGL::RenderSystem *renderer, SDL_Window *window,
            std::string const &path) {
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForVulkan(window);
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

        auto &state = r.set<GuiState>();
        unsigned char *fontData;
        int texWidth, texHeight;
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        state.fontTexture = makeTextureFromData(renderer,
                LLGL::ImageFormat::RGBA, fontData, texWidth, texHeight);
        state.sampler = makeSampler(renderer);
        state.pipeline = guiPipeline::make(renderer, path);
        state.globalData = createUniformBuffer(renderer, Parameters{});

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = state.pipeline.layout;
        resourceHeapDesc.resourceViews.emplace_back(state.globalData);
        resourceHeapDesc.resourceViews.emplace_back(state.sampler);
        resourceHeapDesc.resourceViews.emplace_back(state.fontTexture);
        state.heap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void updateResources(LLGL::RenderSystem *renderer, GuiState &state) {
        ImGuiIO &io = ImGui::GetIO();
        ImDrawData *imDrawData = ImGui::GetDrawData();

        Parameters parameters = {};
        parameters.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        parameters.translate.x = -1.0f - imDrawData->DisplayPos.x * parameters.scale.x;
        parameters.translate.y = -1.0f - imDrawData->DisplayPos.y * parameters.scale.y;

        updateUniformBuffer(renderer, state.globalData, parameters);

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

        if (state.vertexCount != imDrawData->TotalVtxCount) {
            if (state.vertices) {
                renderer->Release(*state.vertices);
            }
            state.vertexCount = imDrawData->TotalVtxCount;
            state.vertices = createVertexBuffer(renderer, state.pipeline.format, vertices);
        } else {
            renderer->WriteBuffer(*state.vertices, 0, vertices.data(),
                    vertices.size() * sizeof(ImDrawVert));
        }

        if (state.indexCount != imDrawData->TotalIdxCount) {
            if (state.indices) {
                renderer->Release(*state.indices);
            }
            state.indexCount = imDrawData->TotalIdxCount;
            state.indices = createIndexBuffer(renderer, indices);
        } else {
            renderer->WriteBuffer(*state.indices, 0, indices.data(),
                    indices.size() * sizeof(ImDrawIdx));
        }
    }

    void renderGui(entt::registry &r, LLGL::RenderSystem *renderer,
            SDL_Window *window, LLGL::CommandBuffer *cmdBuf) {
        auto &state = r.ctx<GuiState>();

        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        if(state.guiCallbacks) {
            state.guiCallbacks(r);
        }
        ImGui::Render();

        updateResources(renderer, state);

        LLGL::Viewport viewport;
        viewport.width = static_cast<float>(width);
        viewport.height = -static_cast<float>(height);
        viewport.x = 0;
        viewport.y = static_cast<float>(height);
        cmdBuf->SetViewport(viewport);

        cmdBuf->SetPipelineState(*state.pipeline.pipeline);
        cmdBuf->SetResourceHeap(*state.heap);

        ImDrawData *imDrawData = ImGui::GetDrawData();
        float fb_width = (imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
        float fb_height = (imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);
        ImVec2 clip_off = imDrawData->DisplayPos;
        ImVec2 clip_scale = imDrawData->FramebufferScale;
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0) {
            cmdBuf->SetVertexBuffer(*state.vertices);
            cmdBuf->SetIndexBuffer(*state.indices);

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

    entt::delegate<void(entt::registry &r)> &guiCallbacks(entt::registry &r) {
        auto &state = r.ctx<GuiState>();
        return state.guiCallbacks;
    }
}