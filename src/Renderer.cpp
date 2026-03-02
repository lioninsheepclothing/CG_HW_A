#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include "stb_image_write.h"

RenderCanvas::RenderCanvas(int w, int h) : pixelWidth(w), pixelHeight(h) {
    colorMap.resize(w * h * 3, 0);
    depthMap.resize(w * h, std::numeric_limits<float>::max());
}

void RenderCanvas::clearCanvas(const glm::vec3& bgColor) {
    for (int i = 0; i < pixelWidth * pixelHeight; ++i) {
        colorMap[i * 3 + 0] = static_cast<unsigned char>(bgColor.r);
        colorMap[i * 3 + 1] = static_cast<unsigned char>(bgColor.g);
        colorMap[i * 3 + 2] = static_cast<unsigned char>(bgColor.b);
        depthMap[i] = std::numeric_limits<float>::max(); // 清空缓冲
    }
}

void RenderCanvas::writePixel(int x, int y, const glm::vec3& color, float depthValue) {
    if (x < 0 || x >= pixelWidth || y < 0 || y >= pixelHeight) return;

    int linearIndex = y * pixelWidth + x;

    if (depthValue < depthMap[linearIndex]) {
        depthMap[linearIndex] = depthValue; // 更新 Z-buffer

        colorMap[linearIndex * 3 + 0] = static_cast<unsigned char>(color.r);
        colorMap[linearIndex * 3 + 1] = static_cast<unsigned char>(color.g);
        colorMap[linearIndex * 3 + 2] = static_cast<unsigned char>(color.b);
    }
}

SoftwareRenderer::SoftwareRenderer(int width, int height)
    : canvas(width, height), screenW(width), screenH(height), matrix_MVP(1.0f) {
}

void SoftwareRenderer::setupCamera(const glm::vec3& cameraPos, const glm::vec3& lookTarget, float fovY) {
    glm::mat4 view = glm::lookAt(cameraPos, lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    float aspect = static_cast<float>(screenW) / static_cast<float>(screenH);
    glm::mat4 projection = glm::perspective(glm::radians(fovY), aspect, 0.1f, 100.0f);

    // MVP = Projection * View * Model
    matrix_MVP = projection * view;
}

glm::vec3 SoftwareRenderer::computeWeights(const glm::vec2& pA, const glm::vec2& pB, const glm::vec2& pC, const glm::vec2& pixelP) {
    float areaTotal = (pB.x - pA.x) * (pC.y - pA.y) - (pB.y - pA.y) * (pC.x - pA.x);
    if (std::abs(areaTotal) < 1e-5f) return glm::vec3(-1.0f); // 极小三角形或退化为线，返回无效权重直接剔除

    float weightA = ((pB.x - pixelP.x) * (pC.y - pixelP.y) - (pB.y - pixelP.y) * (pC.x - pixelP.x)) / areaTotal;
    float weightB = ((pC.x - pixelP.x) * (pA.y - pixelP.y) - (pC.y - pixelP.y) * (pA.x - pixelP.x)) / areaTotal;
    float weightC = 1.0f - weightA - weightB;

    return glm::vec3(weightA, weightB, weightC);
}

void SoftwareRenderer::rasterizeTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color) {
    // 1. 计算三角形在屏幕上的 2D AABB 包围盒
    float minX = std::max(0.0f, std::min({ v0.x, v1.x, v2.x }));
    float maxX = std::min(static_cast<float>(screenW - 1), std::max({ v0.x, v1.x, v2.x }));
    float minY = std::max(0.0f, std::min({ v0.y, v1.y, v2.y }));
    float maxY = std::min(static_cast<float>(screenH - 1), std::max({ v0.y, v1.y, v2.y }));

    // 2. 逐像素扫描该包围盒
    glm::vec2 pA(v0.x, v0.y);
    glm::vec2 pB(v1.x, v1.y);
    glm::vec2 pC(v2.x, v2.y);

    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {

            glm::vec2 pixelPos(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // 取像素中心
            glm::vec3 weights = computeWeights(pA, pB, pC, pixelPos);

            // 像素落在三角形内部
            if (weights.x >= 0.0f && weights.y >= 0.0f && weights.z >= 0.0f) {
                // 利用重心坐标插值获取当前像素的精确深度值
                float currentZ = weights.x * v0.z + weights.y * v1.z + weights.z * v2.z;
                canvas.writePixel(x, y, color, currentZ);
            }
        }
    }
}

void SoftwareRenderer::renderMesh_Baseline(const Mesh& targetMesh) {
    canvas.clearCanvas(glm::vec3(40, 40, 40)); // 深灰色背景

    for (size_t i = 0; i < targetMesh.triangles.size(); ++i) {
        const auto& tri = targetMesh.triangles[i];

        glm::vec3 rawV0 = targetMesh.vertices[tri.v0].position;
        glm::vec3 rawV1 = targetMesh.vertices[tri.v1].position;
        glm::vec3 rawV2 = targetMesh.vertices[tri.v2].position;

        // 处理坐标空间的透视投影和视口映射
        auto transformToScreen = [&](const glm::vec3& v) -> glm::vec3 {
            glm::vec4 clipPos = matrix_MVP * glm::vec4(v, 1.0f);
            glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

            // 映射到屏幕坐标
            float screenX = (ndcPos.x + 1.0f) * 0.5f * screenW;
            float screenY = (1.0f - ndcPos.y) * 0.5f * screenH; // Y轴翻转
            return glm::vec3(screenX, screenY, ndcPos.z);
            };

        glm::vec3 sV0 = transformToScreen(rawV0);
        glm::vec3 sV1 = transformToScreen(rawV1);
        glm::vec3 sV2 = transformToScreen(rawV2);

        float shade = 100.0f + (i % 155);
        glm::vec3 faceColor(shade, shade, shade);

        rasterizeTriangle(sV0, sV1, sV2, faceColor);
    }
}

bool SoftwareRenderer::exportToImage(const std::string& outputPath) {
    return stbi_write_png(outputPath.c_str(), canvas.pixelWidth, canvas.pixelHeight, 3, canvas.colorMap.data(), canvas.pixelWidth * 3) != 0;
}