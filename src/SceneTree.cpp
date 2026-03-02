#include "SceneTree.h"
#include <algorithm>
#include <iostream>
#include "Renderer.h"

SceneTreeBuilder::SceneTreeBuilder() : targetMesh(nullptr) {}

BoundingBox SceneTreeBuilder::computeClusterBounds(const std::vector<int>& faceList) {
    BoundingBox clusterBox;
    for (int faceIdx : faceList) {
        const Triangle& tri = targetMesh->triangles[faceIdx];
        clusterBox.expand(targetMesh->vertices[tri.v0].position);
        clusterBox.expand(targetMesh->vertices[tri.v1].position);
        clusterBox.expand(targetMesh->vertices[tri.v2].position);
    }
    return clusterBox;
}

void SceneTreeBuilder::buildTree(const Mesh& mesh) {
    targetMesh = &mesh;
    treeNodes.clear();

    // 初始状态
    std::vector<int> allFaces(mesh.triangles.size());
    for (size_t i = 0; i < mesh.triangles.size(); ++i) {
        allFaces[i] = static_cast<int>(i);
    }

    std::cout << ">> Initiating spatial partitioning for " << allFaces.size() << " faces..." << std::endl;
    int rootNode = partitionSpace(allFaces, 0);
    std::cout << ">> Tree build complete. Total spatial zones generated: " << treeNodes.size() << std::endl;
}

int SceneTreeBuilder::partitionSpace(std::vector<int>& currentFaces, int depth) {
    int currentNodeIdx = static_cast<int>(treeNodes.size());
    treeNodes.push_back(SpatialZone{});

    // 更新当前节点的包围盒
    treeNodes[currentNodeIdx].zoneBounds = computeClusterBounds(currentFaces);
    treeNodes[currentNodeIdx].leftChildID = -1;
    treeNodes[currentNodeIdx].rightChildID = -1;

    // 终止条件
    const int LEAF_CAPACITY = 4;
    const int MAX_DEPTH = 25;

    if (currentFaces.size() <= LEAF_CAPACITY || depth > MAX_DEPTH) {
        treeNodes[currentNodeIdx].faceIndices = currentFaces;
        return currentNodeIdx;
    }

    glm::vec3 extents = treeNodes[currentNodeIdx].zoneBounds.maxPoint - treeNodes[currentNodeIdx].zoneBounds.minPoint;
    int splitAxis = 0; // 0:x, 1:y, 2:z
    if (extents.y > extents.x && extents.y > extents.z) splitAxis = 1;
    else if (extents.z > extents.x && extents.z > extents.y) splitAxis = 2;

    float splitLine = treeNodes[currentNodeIdx].zoneBounds.minPoint[splitAxis] + extents[splitAxis] * 0.5f;

    std::vector<int> leftList;
    std::vector<int> rightList;

    // 遍历当前节点的所有面片，按质心位置分发到左右两个新数组
    for (int faceIdx : currentFaces) {
        const Triangle& tri = targetMesh->triangles[faceIdx];
        glm::vec3 v0 = targetMesh->vertices[tri.v0].position;
        glm::vec3 v1 = targetMesh->vertices[tri.v1].position;
        glm::vec3 v2 = targetMesh->vertices[tri.v2].position;

        // 计算面片质心的切割轴坐标
        float centroidPos = (v0[splitAxis] + v1[splitAxis] + v2[splitAxis]) / 3.0f;

        if (centroidPos < splitLine) {
            leftList.push_back(faceIdx);
        }
        else {
            rightList.push_back(faceIdx);
        }
    }

    // 如果切分后某一边全空，强制一分为二
    if (leftList.empty() || rightList.empty()) {
        size_t half = currentFaces.size() / 2;
        leftList.assign(currentFaces.begin(), currentFaces.begin() + half);
        rightList.assign(currentFaces.begin() + half, currentFaces.end());
    }

    int leftID = partitionSpace(leftList, depth + 1);
    int rightID = partitionSpace(rightList, depth + 1);

    treeNodes[currentNodeIdx].leftChildID = leftID;
    treeNodes[currentNodeIdx].rightChildID = rightID;

    return currentNodeIdx;
}

// 提取原先在 for 循环中低效的 Lambda 函数
glm::vec3 SoftwareRenderer::transformToScreen(const glm::vec3& v) {
    glm::vec4 clipPos = matrix_MVP * glm::vec4(v, 1.0f);
    // 防除零与简单的近平面标记
    if (clipPos.w <= 0.0f) return glm::vec3(0.0f, 0.0f, -1.0f);

    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
    float screenX = (ndcPos.x + 1.0f) * 0.5f * screenW;
    float screenY = (1.0f - ndcPos.y) * 0.5f * screenH;
    return glm::vec3(screenX, screenY, ndcPos.z);
}

// 空间划分树：AABB 视锥体剔除测试
bool SoftwareRenderer::isAABBVisible(const BoundingBox& bbox) {
    glm::vec3 corners[8] = {
        glm::vec3(bbox.minPoint.x, bbox.minPoint.y, bbox.minPoint.z),
        glm::vec3(bbox.maxPoint.x, bbox.minPoint.y, bbox.minPoint.z),
        glm::vec3(bbox.minPoint.x, bbox.maxPoint.y, bbox.minPoint.z),
        glm::vec3(bbox.maxPoint.x, bbox.maxPoint.y, bbox.minPoint.z),
        glm::vec3(bbox.minPoint.x, bbox.minPoint.y, bbox.maxPoint.z),
        glm::vec3(bbox.maxPoint.x, bbox.minPoint.y, bbox.maxPoint.z),
        glm::vec3(bbox.minPoint.x, bbox.maxPoint.y, bbox.maxPoint.z),
        glm::vec3(bbox.maxPoint.x, bbox.maxPoint.y, bbox.maxPoint.z)
    };

    int outOfBounds[6] = { 0 }; // 记录八个顶点在6个裁剪面外的数量
    for (int i = 0; i < 8; ++i) {
        glm::vec4 clipPos = matrix_MVP * glm::vec4(corners[i], 1.0f);
        if (clipPos.x < -clipPos.w) outOfBounds[0]++;
        if (clipPos.x > clipPos.w) outOfBounds[1]++;
        if (clipPos.y < -clipPos.w) outOfBounds[2]++;
        if (clipPos.y > clipPos.w) outOfBounds[3]++;
        if (clipPos.z < -clipPos.w) outOfBounds[4]++;
        if (clipPos.z > clipPos.w) outOfBounds[5]++;
    }

    // 如果8个顶点全在某一个面之外，说明包围盒彻底在视线外，剔除不渲染
    for (int i = 0; i < 6; ++i) {
        if (outOfBounds[i] == 8) return false;
    }
    return true;
}

// 递归遍历空间树
void SoftwareRenderer::traverseNode(int nodeID, const std::vector<SpatialZone>& nodes, const Mesh& targetMesh) {
    if (nodeID == -1) return;
    const auto& node = nodes[nodeID];

    // 1. 核心加速逻辑：如果包围盒不可见，直接丢弃整个分支的所有三角形
    if (!isAABBVisible(node.zoneBounds)) return;

    // 2. 到达叶子节点，开始光栅化面片
    if (node.isLeaf()) {
        for (int faceIdx : node.faceIndices) {
            const auto& tri = targetMesh.triangles[faceIdx];
            glm::vec3 sV0 = transformToScreen(targetMesh.vertices[tri.v0].position);
            glm::vec3 sV1 = transformToScreen(targetMesh.vertices[tri.v1].position);
            glm::vec3 sV2 = transformToScreen(targetMesh.vertices[tri.v2].position);

            // 简单剔除跑到摄像机背后的顶点
            if (sV0.z < -0.5f || sV1.z < -0.5f || sV2.z < -0.5f) continue;

            float shade = 100.0f + (faceIdx % 155);
            glm::vec3 faceColor(shade, shade, shade);
            rasterizeTriangle(sV0, sV1, sV2, faceColor);
        }
    }
    // 3. 内部节点，继续向下遍历
    else {
        traverseNode(node.leftChildID, nodes, targetMesh);
        traverseNode(node.rightChildID, nodes, targetMesh);
    }
}

// 阶段三：加速渲染入口
void SoftwareRenderer::renderMesh_Accelerated(const Mesh& targetMesh, const SceneTreeBuilder& bvhTree) {
    canvas.clearCanvas(glm::vec3(40, 40, 40));
    const auto& nodes = bvhTree.getNodes();
    if (nodes.empty()) return;

    // 从根节点(0)开始遍历树
    traverseNode(bvhTree.getRootID(), nodes, targetMesh);
}