#include "resources.hpp"

namespace rise {
    LLGL::Texture *makeTextureFromData(LLGL::RenderSystem *renderer, LLGL::ImageFormat format,
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

    LLGL::Sampler *makeSampler(LLGL::RenderSystem *renderer) {
        LLGL::SamplerDescriptor samplerInfo = {};
        return renderer->CreateSampler(samplerInfo);
    }

}
