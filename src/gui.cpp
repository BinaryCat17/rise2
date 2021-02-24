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
        resources.program = makeProgram(instance->renderer.get(), instance->root + "/shaders/gui",
                imguiVertexFormat());
        resources.pipeline = makeGuiPipeline(instance->renderer.get(), resources.layout,
                resources.program);

        resources.parameters = createUniformBuffer(instance->renderer.get(), Parameters{});

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = resources.layout;
        resourceHeapDesc.resourceViews.emplace_back(resources.parameters);
        resourceHeapDesc.resourceViews.emplace_back(resources.sampler);
        resourceHeapDesc.resourceViews.emplace_back(resources.fontTexture);
        resources.heap = instance->renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void updateResources(LLGL::RenderSystem *renderer, GuiResources *resources) {
        ImGuiIO &io = ImGui::GetIO();
        Parameters parameters = {};
        ImDrawData *imDrawData = ImGui::GetDrawData();
        parameters.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        parameters.translate.x = -1.0f - imDrawData->DisplayPos.x * parameters.scale.x;
        parameters.translate.y = -1.0f - imDrawData->DisplayPos.y * parameters.scale.y;

        updateUniformBuffer(renderer, resources->parameters, parameters);


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

        if (resources->vertexCount != imDrawData->TotalVtxCount) {
            if (resources->vertices) {
                renderer->Release(*resources->vertices);
            }
            resources->vertexCount = imDrawData->TotalVtxCount;
            resources->vertices = createVertexBuffer(renderer, imguiVertexFormat(), vertices);
        } else {
            renderer->WriteBuffer(*resources->vertices, 0, vertices.data(),
                    vertices.size() * sizeof(ImDrawVert));
        }

        if (resources->indexCount != imDrawData->TotalIdxCount) {
            if (resources->indices) {
                renderer->Release(*resources->indices);
            }
            resources->indexCount = imDrawData->TotalIdxCount;
            resources->indices = createIndexBuffer(renderer, indices);
        } else {
            renderer->WriteBuffer(*resources->indices, 0, indices.data(),
                    indices.size() * sizeof(ImDrawIdx));
        }
    }

    template<typename T>
    void menuEntityFloat3(entt::registry &r, entt::entity e, std::string const &name) {
        if (auto position = r.try_get<T>(e)) {
            T inputPos = *position;
            if (ImGui::DragFloat3(name.c_str(), &inputPos.x)) {
                r.replace<T>(e, inputPos);
            }
        }
    }

    template<typename T, typename FnT>
    void writeChapter(entt::registry &r, std::string const &title, FnT &&f) {
        if(ImGui::TreeNode(title.c_str())) {
            for (auto e : r.view<T>()) {
                if (auto name = r.try_get<Name>(e)) {
                    if (ImGui::TreeNode(name->c_str())) {
                        f(r, e);
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    void recordImgui(entt::registry &r) {
        auto instance = r.ctx<Instance*>();
        ImGui::SetNextWindowSize({500, 800});
        ImGui::Begin("entity review");

        if(ImGui::TreeNode("Camera")) {
            menuEntityFloat3<Position>(r, instance->camera.camera, "position");
            menuEntityFloat3<Rotation>(r, instance->camera.camera, "rotation");
            ImGui::TreePop();
        }

        writeChapter<Drawable>(r, "Drawable objects: ", [](entt::registry &r, entt::entity e) {
            menuEntityFloat3<Position>(r, e, "position");
            menuEntityFloat3<Rotation>(r, e, "rotation");
            menuEntityFloat3<Scale>(r, e, "scale");
        });
        writeChapter<PointLight>(r, "Light objects: ", [](entt::registry &r, entt::entity e) {
            menuEntityFloat3<Position>(r, e, "position");

            if (auto pColor = r.try_get<DiffuseColor>(e)) {
                DiffuseColor color = *pColor;
                if (ImGui::ColorEdit3("diffuse color", &color.x, ImGuiColorEditFlags_DisplayRGB)) {
                    r.replace<DiffuseColor>(e, color);
                }
            }

            if (auto pLight = r.try_get<PointLight>(e)) {
                PointLight light = *pLight;
                if (ImGui::DragFloat("distance", &light.distance) ||
                        ImGui::DragFloat("intensity", &light.intensity)) {
                    r.replace<PointLight>(e, light);
                }
            }
        });
        ImGui::End();
    }

    void renderGui(entt::registry &r, LLGL::CommandBuffer *cmdBuf) {
        auto instance = r.ctx<Instance *>();
        auto &resources = r.ctx<GuiResources>();

        ImGui_ImplSDL2_NewFrame(instance->window);
        ImGui::NewFrame();
        recordImgui(r);
        ImGui::Render();

        updateResources(instance->renderer.get(), &resources);

        cmdBuf->SetPipelineState(*resources.pipeline);
        cmdBuf->SetResourceHeap(*resources.heap);

        ImDrawData *imDrawData = ImGui::GetDrawData();
        float fb_width = (imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
        float fb_height = (imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);
        ImVec2 clip_off = imDrawData->DisplayPos;
        ImVec2 clip_scale = imDrawData->FramebufferScale;
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0) {
            cmdBuf->SetVertexBuffer(*resources.vertices);
            cmdBuf->SetIndexBuffer(*resources.indices);

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