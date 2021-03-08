#include "state.hpp"

#define LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>

namespace rise::systems::rendering {
    LLGL::ShaderProgram *createShaderProgram(LLGL::RenderSystem *renderer, std::string const &root,
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
}