#include "pipeline.hpp"
#include "../core/utils.hpp"

namespace rise::systems::rendering::scenePipeline {
    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer) {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{ // camera
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                0,
        }, LLGL::BindingDescriptor{ // material
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                1,
        }, LLGL::BindingDescriptor{ // draw
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                2,
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Sampler,
                0,
                LLGL::StageFlags::FragmentStage,
                3
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Texture,
                LLGL::BindFlags::Sampled,
                LLGL::StageFlags::FragmentStage,
                4},
        };

        return renderer->CreatePipelineLayout(layoutDesc);
    }

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program) {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.shaderProgram = program;
        pipelineDesc.pipelineLayout = layout;
        pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
        pipelineDesc.rasterizer.multiSampleEnabled = true;
        pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Front;
        pipelineDesc.depth.compareOp = LLGL::CompareOp::LessEqual;
        pipelineDesc.depth.testEnabled = true;
        pipelineDesc.depth.writeEnabled = true;

        return renderer->CreatePipelineState(pipelineDesc);
    }
}

namespace rise::systems::rendering {
    void initScenePipeline(flecs::entity, RenderSystem const &renderer, Path const &root,
            VertexFormat &format, PipelineLayout &layout, Pipeline &pipeline) {
        format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        layout = scenePipeline::createLayout(renderer.get());
        auto program = createShaderProgram(renderer.get(),
                std::string(root.file) + "/shaders/scene", format);
        pipeline = scenePipeline::createPipeline(renderer.get(), layout, program);
    }
}