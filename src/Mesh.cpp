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

    // 1. 初始化顶点位置
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        Vertex v;
        v.position = glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
        v.normal = glm::vec3(0.0f, 1.0f, 0.0f); // 默认向上
        bbox.expand(v.position);
        vertices.push_back(v);
    }

    // 2. 解析面片索引，并绑定正确的法线
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            if (fv == 3) {
                Triangle tri;
                for (int k = 0; k < 3; ++k) {
                    auto idx = shapes[s].mesh.indices[index_offset + k];
                    int v_idx = idx.vertex_index;
                    int n_idx = idx.normal_index;

                    // 根据 normal_index 覆盖正确的法线数据
                    if (n_idx >= 0 && n_idx * 3 + 2 < attrib.normals.size()) {
                        vertices[v_idx].normal = glm::vec3(
                            attrib.normals[3 * n_idx + 0],
                            attrib.normals[3 * n_idx + 1],
                            attrib.normals[3 * n_idx + 2]
                        );
                    }

                    if (k == 0) tri.v0 = v_idx;
                    if (k == 1) tri.v1 = v_idx;
                    if (k == 2) tri.v2 = v_idx;
                }
                triangles.push_back(tri);
            }
            index_offset += fv;
        }
    }

    std::cout << "Successfully loaded: " << filepath << std::endl;
    std::cout << "Vertices: " << vertices.size() << ", Triangles: " << triangles.size() << std::endl;
    return true;
}