#include "scene.hpp"

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

    PipelineState create(LLGL::RenderSystem *renderer, std::string const &root) {
        PipelineState data;
        data.format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        data.format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        data.format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        data.layout = createLayout(renderer);
        data.program = createShaderProgram(renderer, root + "/shaders/scene", data.format);
        data.pipeline = createPipeline(renderer, data.layout, data.program);

        return data;
    }
}