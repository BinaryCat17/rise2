#include "pipeline.hpp"
#include "../core/utils.hpp"

namespace rise::systems::rendering::guiPipeline {
    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer) {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage,
                0,
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Sampler,
                0,
                LLGL::StageFlags::FragmentStage,
                1
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Texture,
                LLGL::BindFlags::Sampled,
                LLGL::StageFlags::FragmentStage,
                2},
        };

        return renderer->CreatePipelineLayout(layoutDesc);
    }

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program) {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.shaderProgram = program;
        pipelineDesc.pipelineLayout = layout;
        pipelineDesc.rasterizer.polygonMode = LLGL::PolygonMode::Fill;
        pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Disabled;
        pipelineDesc.rasterizer.frontCCW = true;
        pipelineDesc.rasterizer.multiSampleEnabled = true;
        pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
        pipelineDesc.blend.targets[0] = LLGL::BlendTargetDescriptor{
                true,
                LLGL::BlendOp::SrcAlpha,
                LLGL::BlendOp::InvSrcAlpha,
                LLGL::BlendArithmetic::Add,
                LLGL::BlendOp::InvSrcAlpha,
                LLGL::BlendOp::Zero,
                LLGL::BlendArithmetic::Add,
        };

        return renderer->CreatePipelineState(pipelineDesc);
    }
}

namespace rise::systems::rendering {
    void initGuiPipeline(flecs::entity e) {
        auto& renderer = checkGet<RenderSystem>(e);
        VertexFormat format;
        format.AppendAttribute(LLGL::VertexAttribute{"inPos", LLGL::Format::RG32Float});
        format.AppendAttribute(LLGL::VertexAttribute{"inUV", LLGL::Format::RG32Float});
        format.AppendAttribute(LLGL::VertexAttribute{"inColor", LLGL::Format::RGBA8UNorm});

        auto layout = guiPipeline::createLayout(renderer.get());
        e.set(PipelineLayout{layout});
        auto program = createShaderProgram(renderer.get(),
                std::string(checkGet<Path>(e).file) + "/shaders/gui", format);
        e.set(std::move(format));
        e.set(Pipeline{guiPipeline::createPipeline(renderer.get(), layout, program)});
    }
}
