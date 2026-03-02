#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"

struct RenderCanvas {
    int pixelWidth;
    int pixelHeight;
    std::vector<unsigned char> colorMap;
    std::vector<float> depthMap;         // Z-buffer 数组

    RenderCanvas(int w, int h);
    void clearCanvas(const glm::vec3& bgColor);
    void writePixel(int x, int y, const glm::vec3& color, float depthValue);
};

class SoftwareRenderer {
private:
    RenderCanvas canvas;
    glm::mat4 matrix_MVP;

    // 屏幕空间的宽高范围
    int screenW, screenH;

    void rasterizeTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color);
    glm::vec3 computeWeights(const glm::vec2& pA, const glm::vec2& pB, const glm::vec2& pC, const glm::vec2& pixelP);

public:
    SoftwareRenderer(int width, int height);
    void setupCamera(const glm::vec3& cameraPos, const glm::vec3& lookTarget, float fovY);
    void renderMesh_Baseline(const Mesh& targetMesh);
    bool exportToImage(const std::string& outputPath);
};