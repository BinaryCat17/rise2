#include "shader.hpp"

namespace rise::shader {
    LLGL::VertexFormat Vertex::format() {
        LLGL::VertexFormat fmt;
        fmt.AppendAttribute({"position", LLGL::Format::RGB32Float});
        fmt.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        fmt.AppendAttribute({"texCoord", LLGL::Format::RG32Float});
        return fmt;
    }

    LLGL::PipelineLayout *layout(LLGL::RenderSystem *renderer) {
        unsigned const CameraBinding = 0;
        unsigned const MeshDataBinding = 1;

        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage,
                CameraBinding,
        }, {LLGL::BindingDescriptor{
                LLGL::ResourceType::Buffer,
                LLGL::BindFlags::ConstantBuffer,
                LLGL::StageFlags::VertexStage,
                MeshDataBinding,
        }}
        };

        return renderer->CreatePipelineLayout(layoutDesc);
    }

    LLGL::ShaderProgram* program(LLGL::RenderSystem *renderer, std::string const& root) {
        std::string vertPath = root + "/shader.vert";
        std::string fragPath = root + "/shader.frag";

        LLGL::ShaderDescriptor VSDesc, FSDesc;
        VSDesc = {LLGL::ShaderType::Vertex, vertPath.data()};
        FSDesc = {LLGL::ShaderType::Fragment, fragPath.data()};

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
}


