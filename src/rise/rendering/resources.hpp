#pragma once

#include <LLGL/LLGL.h>

namespace rise {
    template<typename T>
    LLGL::Buffer *createUniformBuffer(LLGL::RenderSystem *renderer, T const &init = {}) {
        LLGL::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.size = sizeof(T);
        uniformBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
        uniformBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
        uniformBufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
        return renderer->CreateBuffer(uniformBufferDesc, &init);
    }

    template<typename T>
    T *mapUniformBuffer(LLGL::RenderSystem *renderer, LLGL::Buffer *buffer) {
        void *pData = renderer->MapBuffer(*buffer, LLGL::CPUAccess::ReadWrite);
        return reinterpret_cast<T *>(pData);
    }

    template<typename T>
    void updateUniformBuffer(LLGL::RenderSystem *renderer, LLGL::Buffer *buffer, T const &v) {
        void *pData = renderer->MapBuffer(*buffer, LLGL::CPUAccess::ReadWrite);
        auto uniformData = reinterpret_cast<T *>(pData);
        *uniformData = v;
        renderer->UnmapBuffer(*buffer);
    }

    template<typename T, typename FnT>
    void mapUniformBuffer(LLGL::RenderSystem *renderer, LLGL::Buffer *buffer, FnT &&f) {
        f(mapUniformBuffer<T>(renderer, buffer));
        renderer->UnmapBuffer(*buffer);
    }

    LLGL::Texture *makeTextureFromData(LLGL::RenderSystem *renderer, LLGL::ImageFormat format,
            void const *data, unsigned width, unsigned height);

    LLGL::Sampler *makeSampler(LLGL::RenderSystem *renderer);


    template<typename T>
    LLGL::Buffer *createVertexBuffer(LLGL::RenderSystem *renderer, LLGL::VertexFormat const &format,
            std::vector<T> const &data) {
        LLGL::VertexFormat vertexFormat = format;
        LLGL::BufferDescriptor VBufferDesc;
        VBufferDesc.size = sizeof(T) * data.size();
        VBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        VBufferDesc.vertexAttribs = vertexFormat.attributes;

        return renderer->CreateBuffer(VBufferDesc, data.data());
    }

    template<typename T>
    LLGL::Buffer *createIndexBuffer(LLGL::RenderSystem *renderer, std::vector<T> const &data) {
        LLGL::Format format = LLGL::Format::Undefined;
        if constexpr(sizeof(T) == sizeof(uint32_t)) {
            format = LLGL::Format::R32UInt;
        } else if (sizeof(T) == sizeof(uint16_t)) {
            format = LLGL::Format::R16UInt;
        } else {
            static_assert("invalid format");
        }

        LLGL::BufferDescriptor IBufferDesc;
        IBufferDesc.size = sizeof(T) * data.size();
        IBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        IBufferDesc.format = format;

        return renderer->CreateBuffer(IBufferDesc, data.data());
    }

    struct TextureRes {
        LLGL::Texture *texture = nullptr;
    };

    TextureRes loadTextureFromDisk(LLGL::RenderSystem *renderer, std::string const &path);

    struct MeshRes {
        LLGL::Buffer *vertices = nullptr;
        LLGL::Buffer *indices = nullptr;
        unsigned numIndices = 0;
    };

    MeshRes loadMeshFromDisk(LLGL::RenderSystem *renderer, std::string const &path,
            LLGL::VertexFormat const &format);

}
