#include <iostream>
#include <string>
#include "Mesh.h"
#include "Renderer.h"
#include "SceneTree.h"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

int main(int argc, char** argv) {
    std::cout << "--- CG Homework Phase 2: Standard Z-Buffer Baseline ---" << std::endl;

    std::string rootPath(PROJECT_ROOT);
    std::string modelPath = (argc > 1) ? argv[1] : rootPath + "/models/cheburashka.obj";
    std::string outImagePath = rootPath + "/phase2_baseline.png";

    // 1. 加载模型
    Mesh mesh;
    if (!mesh.LoadFromOBJ(modelPath)) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return -1;
    }
    std::cout << "Model Loaded. Faces to process: " << mesh.triangles.size() << std::endl;

    SceneTreeBuilder bvhTree;
    bvhTree.buildTree(mesh);

    // 2. 初始化渲染引擎 (800x800 画布)
    SoftwareRenderer engine(800, 800);

    // 3. 设置相机
    // 将相机放在模型正前方的 Z 轴上。你可以根据模型的包围盒(bbox)来调整这里的 (0,0,5)
    glm::vec3 camPos(0.0f, 0.0f, 3.0f);
    glm::vec3 lookAt(0.0f, 0.0f, 0.0f);
    engine.setupCamera(camPos, lookAt, 45.0f);

    std::cout << "Rendering Z-Buffer..." << std::endl;

    // 4. 执行基准软光栅化与 Z-buffer 测试
    /*engine.renderMesh_Baseline(mesh);*/
    engine.renderMesh_Accelerated(mesh, bvhTree);

    // 5. 输出结果
    if (engine.exportToImage(outImagePath)) {
        std::cout << "-> Rendering Complete! Check output at: \n   " << outImagePath << std::endl;
    }
    else {
        std::cerr << "-> Failed to save output image." << std::endl;
    }

    return 0;
}