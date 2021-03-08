#include "mesh.hpp"
#include "systems/rendering/pipeline/scene.hpp"
#include "util.hpp"
#include <tiny_obj_loader.h>

namespace rise::systems::rendering {
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

    MeshResource loadMeshFromDisk(LLGL::RenderSystem *renderer, std::string const &path,
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