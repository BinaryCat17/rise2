#include "pipeline.hpp"
#include "resources.hpp"

#define LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>

namespace rise {
    LLGL::PipelineLayout *makeSceneLayout(LLGL::RenderSystem *renderer) {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{ // global
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                0,
        }, LLGL::BindingDescriptor{ // camera
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                1,
        }, LLGL::BindingDescriptor{ // material
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                2,
        }, LLGL::BindingDescriptor{ // draw
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage,
                3,
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Sampler,
                0,
                LLGL::StageFlags::FragmentStage,
                4
        }, LLGL::BindingDescriptor{
                LLGL::ResourceType::Texture,
                LLGL::BindFlags::Sampled,
                LLGL::StageFlags::FragmentStage,
                5},
        };

        return renderer->CreatePipelineLayout(layoutDesc);
    }

    LLGL::PipelineLayout *makeGuiLayout(LLGL::RenderSystem *renderer) {
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

    LLGL::ShaderProgram *makeProgram(LLGL::RenderSystem *renderer, std::string const &root,
            LLGL::VertexFormat const &format) {
        std::string vertPath = root + "/shader.vert.spv";
        std::string fragPath = root + "/shader.frag.spv";

        LLGL::ShaderDescriptor VSDesc, FSDesc;
        VSDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, vertPath.data());
        FSDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, fragPath.data());

        VSDesc.vertex.inputAttribs = format.attributes;

        LLGL::ShaderProgramDescriptor programDesc;
        programDesc.vertexShader = renderer->CreateShader(VSDesc);
        programDesc.fragmentShader = renderer->CreateShader(FSDesc);

        for (auto shader : {programDesc.vertexShader, programDesc.fragmentShader}) {
            std::string log = shader->GetReport();
            if (!log.empty()) {
                std::cerr << log << std::endl;
            }
        }

        return renderer->CreateShaderProgram(programDesc);
    }

    LLGL::PipelineState *makeScenePipeline(LLGL::RenderSystem *renderer,
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

    LLGL::PipelineState *makeGuiPipeline(LLGL::RenderSystem *renderer,
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

    PipelineData scenePipeline::make(LLGL::RenderSystem *renderer, std::string const &root) {
        PipelineData data;
        data.format.AppendAttribute({"position", LLGL::Format::RGB32Float});
        data.format.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        data.format.AppendAttribute({"texCoord", LLGL::Format::RG32Float});

        data.layout = makeSceneLayout(renderer);
        data.program = makeProgram(renderer, root + "/shaders/scene", data.format);
        data.pipeline = makeScenePipeline(renderer, data.layout, data.program);

        return data;
    }

    PipelineData guiPipeline::make(LLGL::RenderSystem *renderer, std::string const &root) {
        PipelineData data;
        data.format.AppendAttribute(LLGL::VertexAttribute{"inPos", LLGL::Format::RG32Float});
        data.format.AppendAttribute(LLGL::VertexAttribute{"inUV", LLGL::Format::RG32Float});
        data.format.AppendAttribute(LLGL::VertexAttribute{"inColor", LLGL::Format::RGBA8UNorm});

        data.layout = makeGuiLayout(renderer);
        data.program = makeProgram(renderer, root + "/shaders/gui", data.format);
        data.pipeline = makeGuiPipeline(renderer, data.layout, data.program);

        return data;
    }
}