#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <queue>
#include <climits>

// Прототипы функций
void measureReadThroughput(const char* filename, size_t iterations);
void findShortestPath(const std::vector<std::vector<std::pair<int, int>>>& graph);
std::vector<std::vector<std::pair<int, int>>> createVeryComplexGraph(int nodes, int edgesPerNode, int maxWeight);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <benchmark> <iterations> [options]" << std::endl;
        std::cerr << "Available benchmarks:" << std::endl;
        std::cerr << "  io-thpt-read <file> <iterations>    - Measure disk read throughput" << std::endl;
        std::cerr << "  short-path <iterations>             - Find shortest path in predefined graph" << std::endl;
        return 1;
    }

    std::string benchmark = argv[1];
    int iterations = 0;
    const char* filename = nullptr;

    try {
        if (benchmark == "short-path") {
            if (argc != 3) {
                std::cerr << "Usage: " << argv[0] << " short-path <iterations>" << std::endl;
                return 1;
            }
            iterations = std::stoi(argv[2]);
        }
        else if (benchmark == "io-thpt-read") {
            if (argc != 4) {
                std::cerr << "Usage: " << argv[0] << " io-thpt-read <file> <iterations>" << std::endl;
                return 1;
            }
            filename = argv[2];
            iterations = std::stoi(argv[3]);
        }
        else {
            std::cerr << "Unknown benchmark: " << benchmark << std::endl;
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid number of iterations" << std::endl;
        return 1;
    }

    if (iterations <= 0) {
        std::cerr << "Error: Number of iterations must be positive!" << std::endl;
        return 1;
    }

    if (benchmark == "io-thpt-read") {
        measureReadThroughput(filename, iterations);
    } else if (benchmark == "short-path") {
        auto graph = createVeryComplexGraph(10000, 10, 100);

        for (int i = 0; i < iterations; ++i) {
            auto startTime = std::chrono::high_resolution_clock::now();
            findShortestPath(graph);
            auto endTime = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> elapsed = endTime - startTime;
            std::cout << "Iteration " << i + 1 << " completed in " << elapsed.count() << " seconds" << std::endl;
        }
    }

    return 0;
}
