#include "mesh.hpp"
#include "utils.hpp"
#include <tiny_obj_loader.h>


namespace rise::rendering {
    std::pair<std::vector<scenePipeline::Vertex>, std::vector<uint32_t>> loadObjMesh(
            tinyobj::attrib_t const &attrib, std::vector<tinyobj::shape_t> const &shapes) {
        std::vector <scenePipeline::Vertex> vertices;
        std::vector <uint32_t> indices;

        std::unordered_map <scenePipeline::Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (const auto &idx : shape.mesh.indices) {
                scenePipeline::Vertex vertex{};

                vertex.pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                vertex.color.x = attrib.colors[3 * idx.vertex_index + 0];
                vertex.color.y = attrib.colors[3 * idx.vertex_index + 1];
                vertex.color.z = attrib.colors[3 * idx.vertex_index + 2];
                vertex.texCoord = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                };
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

    void regMesh(flecs::entity e) {
        if (!e.has<Path>()) e.set<Path>({"cube.obj"});
        e.set<MeshId>({});

    }

    void initMesh(flecs::entity e, ApplicationRef app, MeshId &id) {
        if (!e.has_trait<Initialized, MeshId>()) {
            id.id = app.ref->id->manager.mesh.states.push_back(std::tuple{MeshState{}});
            e.add_trait<Initialized, MeshId>();
            e.patch<Path>([](auto) {});
        }
    }

    void unregMesh(flecs::entity e) {
        if (e.has_trait<Initialized, MeshId>()) {
            e.remove<MeshId>();
            e.remove_trait<Initialized, MeshId>();
        }
    }

    void removeMesh(flecs::entity, ApplicationRef app, MeshId id) {
        app.ref->id->manager.mesh.toRemove.push_back(id);
    }

    void updateObjMesh(flecs::entity, ApplicationRef ref, MeshId meshId, Path const &path) {
        auto &core = ref.ref->id->core;
        auto &scene = ref.ref->id->scene;
        auto &manager = ref.ref->id->manager;
        auto file = ref.ref.entity().get<Path>()->file + "/models/" + path.file;

        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = true;

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

        auto[vertices, indices] = loadObjMesh(reader.GetAttrib(), reader.GetShapes());
        if (vertices.empty() || indices.empty()) {
            std::cout << "Loading mesh error: " << file << std::endl;
            return;
        }

        MeshState mesh;
        if (mesh.vertices && mesh.indices) {
            core.renderer->Release(*mesh.vertices);
            core.renderer->Release(*mesh.indices);
        }

        mesh.vertices = createVertexBuffer(core.renderer.get(), scene.format, vertices);
        mesh.indices = createIndexBuffer(core.renderer.get(), indices);
        mesh.numIndices = static_cast<uint32_t>(indices.size());

        manager.mesh.toInit.emplace_back(mesh, meshId);
    }

//
//    void updateGltfMesh(flecs::entity, ApplicationRef ref, MeshId meshId, std::string const &file) {
//        auto &core = ref.ref->id->core;
//        auto &scene = ref.ref->id->scene;
//        auto &manager = ref.ref->id->manager;
//
//        tinygltf::Model model;
//        tinygltf::TinyGLTF loader;
//        std::string err;
//        std::string warn;
//
//        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file);
//
//        if (!warn.empty()) {
//            printf("Warn: %s\n", warn.c_str());
//        }
//
//        if (!err.empty()) {
//            printf("Err: %s\n", err.c_str());
//        }
//
//        if (!ret) {
//            printf("Failed to parse glTF\n");
//        }
//
//        for(auto const& mesh : model.meshes) {
//            mesh.
//        }
//
//    }

    void regMeshToModel(flecs::entity e, ApplicationRef app, ModelId model) {
        if (model.id != NullKey) {
            auto &manager = app.ref->id->manager;
            auto &meshes = std::get<eModelMeshes>(manager.model.states.at(model.id)).get();

            auto &prev = *e.get_trait_mut<Previous, MeshId>();
            if (prev.e != flecs::entity(0)) {
                meshes.erase(prev.e.id());
            }

            meshes.insert(e.id());
            prev.e = e;
        }
    }

    void unregMeshFromModel(flecs::entity e, ApplicationRef app, ModelId model) {
        if (model.id != NullKey) {
            auto &manager = app.ref->id->manager;
            auto &models = std::get<eModelMeshes>(manager.model.states.at(model.id)).get();
            models.erase(e.id());
        }
    }

    void importMesh(flecs::world &ecs) {
        ecs.system<>("regMesh", "Mesh").kind(flecs::OnAdd).each(regMesh);
        ecs.system<>("unregMesh", "Mesh").kind(flecs::OnRemove).each(unregMesh);
        ecs.system<const ApplicationRef, MeshId>("initMesh", "!TRAIT | Initialized > MeshId").
                kind(flecs::OnSet).each(initMesh);
        ecs.system<const ApplicationRef, const MeshId>("removeMesh").
                kind(EcsUnSet).each(removeMesh);
        ecs.system<const ApplicationRef, const ModelId>("regMeshToModel",
                "[in] ANY: MeshId, ANY: ModelInitialized").
                kind(flecs::OnSet).each(regMeshToModel);
        ecs.system<const ApplicationRef, const ModelId>("unregMesFromModel",
                "[in] ANY: MeshId, ANY: ModelInitialized").
                kind(EcsUnSet).each(unregMeshFromModel);
        ecs.system<const ApplicationRef, const MeshId, const Path>("updateMesh",
                "Mesh, TRAIT | Initialized > MeshId").kind(flecs::OnSet).each(updateObjMesh);
    }
}
