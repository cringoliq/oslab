#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <climits>

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
