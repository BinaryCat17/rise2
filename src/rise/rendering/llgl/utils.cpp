#include "utils.hpp"

#define LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>
#include <filesystem>

namespace rise::rendering {
    LLGL::Texture *createTextureFromData(LLGL::RenderSystem *renderer, LLGL::ImageFormat format,
            void const *data, unsigned width, unsigned height) {
        LLGL::SrcImageDescriptor imageDesc;
        imageDesc.format = format;
        imageDesc.dataType = LLGL::DataType::UInt8;
        imageDesc.data = data;
        if (format == LLGL::ImageFormat::RGB) {
            imageDesc.dataSize = width * height * 3;
        } else if (format == LLGL::ImageFormat::RGBA) {
            imageDesc.dataSize = width * height * 4;
        } else {
            throw std::runtime_error("undefined image format");
        }

        LLGL::TextureDescriptor texDesc;
        texDesc.type = LLGL::TextureType::Texture2D;
        texDesc.format = LLGL::Format::BGRA8UNorm;
        texDesc.extent = {static_cast<std::uint32_t>(width),
                static_cast<std::uint32_t>(height), 1u};

        return renderer->CreateTexture(texDesc, &imageDesc);
    }

    LLGL::Sampler *createSampler(LLGL::RenderSystem *renderer) {
        LLGL::SamplerDescriptor samplerInfo = {};
        samplerInfo.magFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.minFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.mipMapFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.addressModeU = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.addressModeV = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.addressModeW = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.borderColor = {1, 1, 1, 1};
        return renderer->CreateSampler(samplerInfo);
    }

    LLGL::ShaderProgram *createShaderProgram(LLGL::RenderSystem *renderer, std::string const &root,
            LLGL::VertexFormat const &format) {

        std::string vertPath = root + "/shader.vert.spv";
        LLGL::ShaderProgramDescriptor programDesc;

        if (std::filesystem::exists(vertPath)) {
            auto desc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, vertPath.data());
            desc.vertex.inputAttribs = format.attributes;
            programDesc.vertexShader = renderer->CreateShader(desc);
        }

        std::string fragPath = root + "/shader.frag.spv";
        if (std::filesystem::exists(fragPath)) {
            programDesc.fragmentShader = renderer->CreateShader(
                    LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, fragPath.data()));
        }

        std::string geomPath = root + "/shader.geom.spv";
        if (std::filesystem::exists(geomPath)) {
            programDesc.geometryShader = renderer->CreateShader(
                    LLGL::ShaderDescFromFile(LLGL::ShaderType::Geometry, geomPath.data()));
        }

        for (auto shader : {programDesc.vertexShader, programDesc.fragmentShader, programDesc.geometryShader}) {
            std::string log = shader->GetReport();
            if (!log.empty()) {
                std::cerr << log << std::endl;
            }
        }

        return renderer->CreateShaderProgram(programDesc);
    }
}
