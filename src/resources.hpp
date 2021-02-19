#pragma once

#include <LLGL/LLGL.h>
#include <entt/entt.hpp>

namespace rise {
    struct Instance {
        std::unique_ptr<LLGL::RenderSystem> renderer;
        LLGL::RenderContext *context;
        LLGL::Window *window;
    };

    void initRenderer(entt::registry &r) {
        r.set(createRenderer());


    }
























    template<typename T>
    LLGL::Buffer *createUniformBuffer(LLGL::RenderSystem *renderer, T const &init = {}) {
        LLGL::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.size = sizeof(T);
        uniformBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
        uniformBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
        uniformBufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
        return renderer->CreateBuffer(uniformBufferDesc, &init);
    }

    template<typename T, typename... Types, typename FnT>
    void mapUniformBuffer(LLGL::RenderSystem *renderer, LLGL::Buffer *buffer, FnT &&f,
            Types &&... args) {
        void *pData = renderer->MapBuffer(*buffer, LLGL::CPUAccess::ReadWrite);
        auto uniformData = reinterpret_cast<T *>(pData);
        f(uniformData, args...);
        renderer->UnmapBuffer(*buffer);
    }
}
