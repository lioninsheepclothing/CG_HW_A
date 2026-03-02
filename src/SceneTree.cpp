#include "SceneTree.h"
#include <algorithm>
#include <iostream>

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

    treeNodes[currentNodeIdx].leftChildID = partitionSpace(leftList, depth + 1);
    treeNodes[currentNodeIdx].rightChildID = partitionSpace(rightList, depth + 1);

    return currentNodeIdx;
}