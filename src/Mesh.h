#pragma once
#include "Types.h"
#include <vector>
#include <string>

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    BoundingBox bbox;

    bool LoadFromOBJ(const std::string& filepath);
};