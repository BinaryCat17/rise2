#include "mesh.hpp"
#include "pipeline.hpp"
#include "../core/utils.hpp"
#include <tiny_obj_loader.h>

namespace rise::rendering {
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

    void updateMesh(flecs::entity, CoreState &core, SceneState &scene, MeshRes &mesh,
            Path const &path) {
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = false;
        auto file = core.path + "/models/" + path.file;

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(file, readerConfig)) {
            if (!reader.Error().empty()) {
                std::string err = "TinyObjReader: " + reader.Error();
                std::cerr << err << std::endl;
                return;
            }
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto[vertices, indices] = loadMesh(reader.GetAttrib(), reader.GetShapes());
        if(vertices.empty() || indices.empty()) {
            throw std::runtime_error(std::string("Loading mesh error: ") + file);
        }

        if(mesh.vertices) {
            core.renderer->Release(*mesh.vertices);
        }
        if(mesh.indices) {
            core.renderer->Release(*mesh.indices);
        }

        mesh.vertices = createVertexBuffer(core.renderer.get(), scene.format, vertices);
        mesh.indices = createIndexBuffer(core.renderer.get(), indices);
        mesh.numIndices = static_cast<uint32_t>(indices.size());
    }
}
