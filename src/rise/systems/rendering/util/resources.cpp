#include "resources.hpp"

namespace rise::systems::rendering {
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
}
