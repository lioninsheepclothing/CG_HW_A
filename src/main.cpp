#include <iostream>
#include <vector>
#include "Mesh.h"

#include "stb_image_write.h"

void TestImageOutput() {
    const int width = 800;
    const int height = 600;
    const int channels = 3;

    std::vector<unsigned char> pixels(width * height * channels);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;
            pixels[index + 0] = 50;
            pixels[index + 1] = 100;
            pixels[index + 2] = 150;
        }
    }

    if (stbi_write_png("test_output.png", width, height, channels, pixels.data(), width * channels)) {
        std::cout << "Successfully wrote test_output.png" << std::endl;
    }
    else {
        std::cerr << "Failed to write image." << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "--- CG Homework: Hierarchical Z-Buffer ---" << std::endl;

    TestImageOutput();

    Mesh mesh;
    // Ä¬ÈÏÃüÁîÐÐ
    std::string modelPath = (argc > 1) ? argv[1] : "models/cheburashka.obj";

    if (mesh.LoadFromOBJ(modelPath)) {
        std::cout << "Phase 1 Data Pipeline setup complete!" << std::endl;
    }
    else {
        std::cerr << "Failed to load OBJ: " << modelPath << std::endl;
    }

    return 0;
}