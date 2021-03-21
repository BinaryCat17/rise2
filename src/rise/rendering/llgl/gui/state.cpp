#include "state.hpp"
#include "pipeline.hpp"
#include <backends/imgui_impl_sdl.h>
#include "rendering/imgui.hpp"
#include "../core/state.hpp"
#include "../core/utils.hpp"
#include "../module.hpp"

namespace rise::rendering {
    void configImGui() {
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    }

    void initGuiPipeline(CoreState &core, GuiState &gui) {
        gui.format.AppendAttribute(LLGL::VertexAttribute{"inPos", LLGL::Format::RG32Float});
        gui.format.AppendAttribute(LLGL::VertexAttribute{"inUV", LLGL::Format::RG32Float});
        gui.format.AppendAttribute(LLGL::VertexAttribute{"inColor", LLGL::Format::RGBA8UNorm});

        gui.layout = guiPipeline::createLayout(core.renderer.get());
        auto program = createShaderProgram(core.renderer.get(),
                core.root + "/shaders/gui", gui.format);
        gui.pipeline = guiPipeline::createPipeline(core.renderer.get(), gui.layout, program);
    }

    void initGuiState(flecs::entity e, CoreState &core, GuiState &gui) {
        auto renderer = core.renderer.get();
        initGuiPipeline(core, gui);

        auto context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        e.get_mut<CoreState>()->sharedState.set<GuiContext>(GuiContext{context});

        ImGui_ImplSDL2_InitForVulkan(core.window);
        configImGui();

        unsigned char *fontData;
        int texWidth, texHeight;
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        auto fontTexture = createTextureFromData(renderer, LLGL::ImageFormat::RGBA,
                fontData, texWidth, texHeight);

        gui.uniform = createUniformBuffer(renderer, guiPipeline::Global{});

        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        resourceHeapDesc.pipelineLayout = gui.layout;
        resourceHeapDesc.resourceViews.emplace_back(gui.uniform);
        resourceHeapDesc.resourceViews.emplace_back(core.sampler);
        resourceHeapDesc.resourceViews.emplace_back(fontTexture);
        gui.heap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void regGuiState(flecs::entity e) {
        e.set<GuiState>({});
    }

    void importGuiState(flecs::world &ecs) {
        ecs.system<>("regGuiState", "Application").
                kind(flecs::OnAdd).each(regGuiState);

        ecs.system<CoreState, GuiState>("initGuiState", "OWNED:Application").
                kind(flecs::OnSet).each(initGuiState);
    }
}