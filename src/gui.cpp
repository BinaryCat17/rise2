#include "gui.hpp"
#include "rendering.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include "resources.hpp"
#include "pipeline.hpp"

namespace rise {
    struct GuiResources {
        LLGL::Texture *fontTexture = nullptr;
        LLGL::Sampler *sampler = nullptr;
        LLGL::Buffer *parameters = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
        LLGL::VertexFormat *format = nullptr;
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::ShaderProgram *program = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        unsigned vertexCount = 0;
        unsigned indexCount = 0;
    };

    struct Parameters {
        alignas(8) glm::vec2 scale = glm::vec2(1.f, 1.f);
        alignas(8) glm::vec2 translate = glm::vec2(0.f, 0.f);
    };

    void initGui(entt::registry &r) {
        auto instance = r.ctx<Instance *>();

        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        // Dimensions
        ImGuiIO &io = ImGui::GetIO();
        auto resolution = instance->context->GetResolution();
        io.DisplaySize = ImVec2(static_cast<float>(resolution.width),
                static_cast<float>(resolution.height));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        auto &resources = r.set<GuiResources>();

        unsigned char *fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        resources.fontTexture = makeTextureFromData(instance->renderer.get(),
                LLGL::ImageFormat::RGBA, fontData, texWidth, texHeight);
        resources.sampler = makeSampler(instance->renderer.get());
        resources.layout = makeGuiLayout(instance->renderer.get());
        resources.program = makeProgram(instance->renderer.get(), instance->root + "/shaders/gui");
        resources.pipeline = makePipeline(instance->renderer.get(), instance->layout,
                instance->program);

        resources.parameters = createUniformBuffer(instance->renderer.get(), Parameters{});

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = resources.layout;
        resourceHeapDesc.resourceViews.emplace_back(resources.parameters);
        resourceHeapDesc.resourceViews.emplace_back(resources.sampler);
        resourceHeapDesc.resourceViews.emplace_back(resources.fontTexture);
        resources.heap = instance->renderer->CreateResourceHeap(resourceHeapDesc);
    }

    LLGL::VertexFormat imguiVertexFormat() {
        LLGL::VertexFormat format;
        format.AppendAttribute(LLGL::VertexAttribute{
                "inPos", LLGL::Format::RG32Float,
        });
        format.AppendAttribute(LLGL::VertexAttribute{
                "inUV", LLGL::Format::RG32Float,
        });
        format.AppendAttribute(LLGL::VertexAttribute{
                "inColor", LLGL::Format::RGBA8UNorm,
        });
        return format;
    }

    void updateResources(LLGL::RenderSystem *renderer, GuiResources *resources) {
        ImDrawData *imDrawData = ImGui::GetDrawData();

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

        std::cout << "Total vertices: " << vertices.size() << std::endl;
        std::cout << "Total indices: " << indices.size() << std::endl;

        if (resources->vertexCount != imDrawData->TotalVtxCount) {
            if (resources->vertices) {
                renderer->Release(*resources->vertices);
            }
            resources->vertexCount = imDrawData->TotalVtxCount;
            resources->vertices = createVertexBuffer(renderer, imguiVertexFormat(), vertices);
        }

        if (resources->indexCount != imDrawData->TotalIdxCount) {
            if (resources->indices) {
                renderer->Release(*resources->indices);
            }
            resources->indexCount = imDrawData->TotalIdxCount;
            resources->indices = createIndexBuffer(renderer, indices);
        }
    }

    void renderGui(entt::registry &r, LLGL::CommandBuffer *cmdBuf) {
        auto instance = r.ctx<Instance *>();
        auto &resources = r.ctx<GuiResources>();

        ImGui_ImplSDL2_NewFrame(instance->window);
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        updateResources(instance->renderer.get(), &resources);

        cmdBuf->SetPipelineState(*resources.pipeline);
        cmdBuf->SetResourceHeap(*resources.heap);

        ImDrawData *imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0) {
            cmdBuf->SetVertexBuffer(*resources.vertices);
            cmdBuf->SetIndexBuffer(*resources.indices);

            for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imDrawData->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                    const ImDrawCmd *pCmd = &cmd_list->CmdBuffer[j];
                    LLGL::Scissor scissor;
                    scissor.x = std::max((int32_t) (pCmd->ClipRect.x), 0);
                    scissor.y = std::max((int32_t) (pCmd->ClipRect.y), 0);
                    scissor.width = static_cast<int32_t>((pCmd->ClipRect.z - pCmd->ClipRect.x));
                    scissor.height = static_cast<int32_t>((pCmd->ClipRect.w - pCmd->ClipRect.y));
                    cmdBuf->SetScissor(scissor);
                    cmdBuf->DrawIndexed(pCmd->ElemCount, indexOffset);
                    indexOffset += static_cast<int32_t>(pCmd->ElemCount);
                }
                vertexOffset += cmd_list->VtxBuffer.Size;
            }
        }
    }
}