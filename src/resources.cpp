#include "resources.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "pipeline.hpp"
#include <tiny_obj_loader.h>

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
        samplerInfo.magFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.minFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.mipMapFilter = LLGL::SamplerFilter::Linear;
        samplerInfo.addressModeU = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.addressModeV = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.addressModeW = LLGL::SamplerAddressMode::Clamp;
        samplerInfo.borderColor = {1, 1, 1, 1};
        return renderer->CreateSampler(samplerInfo);
    }

    TextureRes loadTextureFromDisk(LLGL::RenderSystem *renderer, std::string const &path) {
        int texWidth = 0, texHeight = 0, texComponents = 0;

        unsigned char *imageBuffer = stbi_load(path.c_str(),
                &texWidth, &texHeight, &texComponents, 0);
        if (!imageBuffer)
            throw std::runtime_error("failed to load image from file: " + path);

        auto texture = makeTextureFromData(renderer,
                (texComponents == 4 ? LLGL::ImageFormat::RGBA : LLGL::ImageFormat::RGB),
                imageBuffer, texWidth, texHeight);

        stbi_image_free(imageBuffer);

        return {texture};
    }

    std::pair<std::vector<scenePipeline::Vertex>, std::vector<uint32_t>> loadMesh(
            tinyobj::attrib_t const &attrib, std::vector<tinyobj::shape_t> const &shapes) {
        std::vector<scenePipeline::Vertex> vertices;
        std::vector<uint32_t> indices;

        std::unordered_map<scenePipeline::Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (const auto &idx : shape.mesh.indices) {
                scenePipeline::Vertex vertex{};

                vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                vertex.texCoord.y = attrib.texcoords[2 * idx.texcoord_index + 1];

                vertices.push_back(vertex);

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);

            }
        }

        return {vertices, indices};
    }

    MeshRes loadMeshFromDisk(LLGL::RenderSystem *renderer, std::string const &path,
            LLGL::VertexFormat const &format) {
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = false;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path, readerConfig)) {
            if (!reader.Error().empty()) {
                std::string err = "TinyObjReader: " + reader.Error();
                throw std::runtime_error(err);
            }
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto[vertices, indices] = loadMesh(reader.GetAttrib(), reader.GetShapes());

        auto vertexBuffer = createVertexBuffer(renderer, format, vertices);
        auto indexBuffer = createIndexBuffer(renderer, indices);

        return {vertexBuffer, indexBuffer, static_cast<uint32_t>(indices.size())};
    }
}