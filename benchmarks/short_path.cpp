#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <cstdlib>   // Для генерации случайных чисел
#include <ctime>     // Для инициализации генератора случайных чисел

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
