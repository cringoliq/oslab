#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <queue>
#include <mutex>
#include <limits>
#include <chrono>
#include <cstdlib>
#include <ctime>

// Прототипы функций

void measureReadThroughput(const char* filename, size_t iterations) {
    const size_t blockSize = 8 * 1024; // 8K блок
    const size_t numBlocks = 1000; // Уменьшаем количество блоков для теста

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "File size: " << fileSize / 1024 / 1024 << " MB" << std::endl;

    if (fileSize < blockSize) {
        std::cerr << "File is too small to read a single block." << std::endl;
        return;
    }

    std::vector<char> buffer(blockSize);
    size_t totalBytesRead = 0;

    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        file.clear(); // Сброс состояния потока
        file.seekg(0, std::ios::beg); // Возвращаемся к началу файла

        auto startTime = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < numBlocks; ++i) {
            file.read(buffer.data(), blockSize);
            if (!file) {
                if (file.eof()) {
                    std::cerr << "End of file reached unexpectedly." << std::endl;
                } else {
                    std::cerr << "Error reading file." << std::endl;
                }
                return;
            }
            totalBytesRead += blockSize;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        double throughput = (totalBytesRead / 1024.0 / 1024.0) / elapsed.count(); // MB/s

        std::cout << "Iteration " << iteration + 1 << " throughput: " << throughput << " MB/s" << std::endl;
    }

    file.close();
}



void ioThptReadWorker(const char* filename, size_t iterations) {
    measureReadThroughput(filename, iterations);
}

void findShortestPath(const std::vector<std::vector<std::pair<int, int>>>& graph) {
    const int INF = std::numeric_limits<int>::max();
    // Случайный выбор start и end в пределах графа
    int start = std::rand() % graph.size();
    int end = std::rand() % graph.size();

    while (start == end) {
        end = std::rand() % graph.size();  // Перезапускаем, если start и end совпадают

    std::vector<int> distances(graph.size(), INF);
    distances[start] = 0;

    using Node = std::pair<int, int>; // {расстояние, вершина}
    std::priority_queue<Node, std::vector<Node>, std::greater<>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [currentDistance, currentVertex] = pq.top();
        pq.pop();

        if (currentDistance > distances[currentVertex]) continue;

        for (const auto& [neighbor, weight] : graph[currentVertex]) {
            int newDistance = currentDistance + weight;
            if (newDistance < distances[neighbor]) {
                distances[neighbor] = newDistance;
                pq.push({newDistance, neighbor});
            }
        }
    }

    if (distances[end] == INF) {
        std::cout << "No path found from " << start << " to " << end << std::endl;
    } else {
        std::cout << "Shortest path from " << start << " to " << end << ": " << distances[end] << std::endl;
    }
}
}

std::vector<std::vector<std::pair<int, int>>> createVeryComplexGraph(int nodes, int edgesPerNode, int maxWeight) {
    std::vector<std::vector<std::pair<int, int>>> graph(nodes);

    // Инициализируем генератор случайных чисел
    std::srand(std::time(nullptr));

    for (int i = 0; i < nodes; ++i) {
        for (int j = 0; j < edgesPerNode; ++j) {
            int neighbor = std::rand() % nodes;       // Случайный узел
            int weight = 1 + std::rand() % maxWeight; // Случайный вес (1 до maxWeight)

            // Избегаем самопетель
            if (neighbor != i) {
                graph[i].emplace_back(neighbor, weight);
            }
        }
    }

    return graph;
}



void shortPathWorker(const std::vector<std::vector<std::pair<int, int>>>& graph, size_t iterations) {
    for (size_t i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        findShortestPath(graph);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Thread iteration " << i + 1 << " completed in " << elapsed.count() << " seconds\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <benchmark> <threads> <iterations> [options]\n";
        std::cerr << "Available benchmarks:\n";
        std::cerr << "  io-thpt-read <file> <iterations> - Disk read throughput with threads\n";
        std::cerr << "  short-path <iterations>          - Shortest path with threads\n";
        return 1;
    }

    std::string benchmark = argv[1];
    int threads = std::stoi(argv[2]);
    size_t iterations = std::stoull(argv[3]);

    if (threads <= 0 || iterations <= 0) {
        std::cerr << "Error: Number of threads and iterations must be positive.\n";
        return 1;
    }

    if (benchmark == "io-thpt-read") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " io-thpt-read <threads> <iterations> <file>\n";
            return 1;
        }
        const char* filename = argv[4];

        std::vector<std::thread> workers;
        for (int i = 0; i < threads; ++i) {
            workers.emplace_back(ioThptReadWorker, filename, iterations);
        }

        for (auto& worker : workers) {
            worker.join();
        }
    } else if (benchmark == "short-path") {
        auto graph = createVeryComplexGraph(10000, 10, 100);

        std::vector<std::thread> workers;
        for (int i = 0; i < threads; ++i) {
            workers.emplace_back(shortPathWorker, std::ref(graph), iterations);
        }

        for (auto& worker : workers) {
            worker.join();
        }
    } else {
        std::cerr << "Unknown benchmark: " << benchmark << "\n";
        return 1;
    }

    return 0;
}

