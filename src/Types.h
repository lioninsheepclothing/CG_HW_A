#pragma once
#include <glm/glm.hpp>

enum class RenderMode {
    BASELINE,        // 扫描线 Z-buffer
    SIMPLE_HZB,      // 简单模式：层次 Z-buffer
    COMPLETE_HZB     // 完整模式：层次 Z-buffer + BVH
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Triangle {
    int v0, v1, v2;
};

struct BoundingBox {
    glm::vec3 minPoint;
    glm::vec3 maxPoint;

    BoundingBox() :
        minPoint(glm::vec3(1e9f)),
        maxPoint(glm::vec3(-1e9f)) {
    }

    void expand(const glm::vec3& point) {
        minPoint = glm::min(minPoint, point);
        maxPoint = glm::max(maxPoint, point);
    }
};