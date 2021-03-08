#include "gui.hpp"

namespace rise::systems::rendering::guiPipeline {
    LLGL::PipelineLayout* createLayout(LLGL::RenderSystem* renderer) {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = { LLGL::BindingDescriptor{
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

    LLGL::PipelineState* createPipeline(LLGL::RenderSystem* renderer,
            LLGL::PipelineLayout* layout, LLGL::ShaderProgram* program) {
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


    PipelineState create(LLGL::RenderSystem* renderer, std::string const& root) {
        PipelineState data;
        data.format.AppendAttribute(LLGL::VertexAttribute{ "inPos", LLGL::Format::RG32Float });
        data.format.AppendAttribute(LLGL::VertexAttribute{ "inUV", LLGL::Format::RG32Float });
        data.format.AppendAttribute(LLGL::VertexAttribute{ "inColor", LLGL::Format::RGBA8UNorm });

        data.layout = createLayout(renderer);
        data.program = createShaderProgram(renderer, root + "/shaders/gui", data.format);
        data.pipeline = createPipeline(renderer, data.layout, data.program);

        return data;
    }
}
