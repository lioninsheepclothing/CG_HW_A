#pragma once
#include <vector>
#include <memory>
#include "Types.h"
#include "Mesh.h"

struct SpatialZone {
    BoundingBox zoneBounds;

    // ÍØÆË½á¹¹
    int leftChildID;
    int rightChildID;

    std::vector<int> faceIndices;

    bool isLeaf() const {
        return leftChildID == -1 && rightChildID == -1;
    }
};

class SceneTreeBuilder {
private:
    std::vector<SpatialZone> treeNodes;
    const Mesh* targetMesh;

    int partitionSpace(std::vector<int>& currentFaces, int depth);
    BoundingBox computeClusterBounds(const std::vector<int>& faceList);

public:
    SceneTreeBuilder();

    void buildTree(const Mesh& mesh);
    const std::vector<SpatialZone>& getNodes() const { return treeNodes; }
    int getRootID() const { return treeNodes.empty() ? -1 : 0; }
};