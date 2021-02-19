#include "mesh.hpp"
#include <tiny_obj_loader.h>

namespace rise {
    std::pair<std::vector<Vertex>, std::vector<uint32_t>> loadMesh(
            tinyobj::attrib_t const &attrib, std::vector<tinyobj::shape_t> const &shapes) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (const auto &shape : shapes) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    Vertex vertex{};
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                    vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                    vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                    vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                    vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                    vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                    vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vertex.texCoord.y = attrib.texcoords[2 * idx.texcoord_index + 1];
                    vertex.color.r = attrib.colors[3 * idx.vertex_index + 0];
                    vertex.color.g = attrib.colors[3 * idx.vertex_index + 1];
                    vertex.color.b = attrib.colors[3 * idx.vertex_index + 2];

                    vertices.push_back(vertex);
                    std::copy(shape.points.indices.begin(), shape.points.indices.end(),
                            std::back_inserter(indices));
                }
                index_offset += fv;
            }
        }

        return {vertices, indices};
    }

    LLGL::VertexFormat getVertexFormat() {
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({"position", LLGL::Format::RGB32Float});
        vertexFormat.AppendAttribute({"normal", LLGL::Format::RGB32Float});
        vertexFormat.AppendAttribute({"texCoord", LLGL::Format::RG32Float});
        vertexFormat.AppendAttribute({"color", LLGL::Format::RGB32Float});
        return vertexFormat;
    }

    Mesh loadMesh(LLGL::RenderSystem *renderer, std::string const &path) {
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = true;
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

        LLGL::VertexFormat vertexFormat = getVertexFormat();
        LLGL::BufferDescriptor VBufferDesc;
        VBufferDesc.size = sizeof(Vertex) * vertices.size();
        VBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        VBufferDesc.vertexAttribs = vertexFormat.attributes;

        auto vertexBuffer = renderer->CreateBuffer(VBufferDesc, vertices.data());

        LLGL::BufferDescriptor IBufferDesc;
        IBufferDesc.size = sizeof(uint32_t) * indices.size();
        IBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        IBufferDesc.format = LLGL::Format::R32UInt;

        auto indexBuffer = renderer->CreateBuffer(IBufferDesc, indices.data());

        return Mesh{vertexFormat, vertexBuffer, indexBuffer,
                static_cast<uint32_t>(indices.size())};
    }

    void drawMesh(LLGL::CommandBuffer *cmdBuf, Mesh& model) {
        cmdBuf->SetVertexBuffer(*model.vertices);
        cmdBuf->SetIndexBuffer(*model.indices);
        cmdBuf->DrawIndexed(model.numIndices, 0);
    }
}