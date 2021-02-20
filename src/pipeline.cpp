#include "pipeline.hpp"
#define LLGL_ENABLE_UTILITY
#include <LLGL/Utility.h>

namespace rise {
    LLGL::VertexFormat Vertex::format() {
        LLGL::VertexFormat fmt;
        fmt.AppendAttribute({"position", LLGL::Format::RGB32Float});
        fmt.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        fmt.AppendAttribute({"texCoord", LLGL::Format::RG32Float});
        return fmt;
    }

    LLGL::PipelineLayout *makeLayout(LLGL::RenderSystem *renderer) {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage,
                0,
        }, {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage,
                1,
        }}
        };

        return renderer->CreatePipelineLayout(layoutDesc);
    }

    LLGL::ShaderProgram* makeProgram(LLGL::RenderSystem *renderer, std::string const& root) {
        std::string vertPath = root + "/shader.vert.spv";
        std::string fragPath = root + "/shader.frag.spv";

        LLGL::ShaderDescriptor VSDesc, FSDesc;
        VSDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, vertPath.data());
        FSDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, fragPath.data());

        VSDesc.vertex.inputAttribs = Vertex::format().attributes;

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

    LLGL::PipelineState* makePipeline(LLGL::RenderSystem* renderer, LLGL::PipelineLayout* layout,
      LLGL::ShaderProgram* program) {
              LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.shaderProgram = program;
        pipelineDesc.pipelineLayout = layout;
        pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
        pipelineDesc.rasterizer.multiSampleEnabled = true;
        pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Front;
        pipelineDesc.depth.compareOp = LLGL::CompareOp::LessEqual;
        pipelineDesc.depth.testEnabled = true;
        pipelineDesc.depth.writeEnabled = true;

        return renderer->CreatePipelineState(pipelineDesc);
    }
}


