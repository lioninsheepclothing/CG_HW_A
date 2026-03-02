#include "Mesh.h"
#include <iostream>

#include "tiny_obj_loader.h"

bool Mesh::LoadFromOBJ(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

    if (!warn.empty()) std::cout << "OBJ Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "OBJ Error: " << err << std::endl;
    if (!ret) return false;

    // 解析顶点位置和法线
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        Vertex v;
        v.position = glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
        bbox.expand(v.position);

        // 防止越界
        if (!attrib.normals.empty() && (i + 2) < attrib.normals.size()) {
            v.normal = glm::vec3(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]);
        }
        else {
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        vertices.push_back(v);
    }

    // 解析面片索引
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            if (fv == 3) {
                Triangle tri;
                tri.v0 = shapes[s].mesh.indices[index_offset + 0].vertex_index;
                tri.v1 = shapes[s].mesh.indices[index_offset + 1].vertex_index;
                tri.v2 = shapes[s].mesh.indices[index_offset + 2].vertex_index;
                triangles.push_back(tri);
            }
            index_offset += fv;
        }
    }

    std::cout << "Successfully loaded: " << filepath << std::endl;
    std::cout << "Vertices: " << vertices.size() << ", Triangles: " << triangles.size() << std::endl;
    return true;
}