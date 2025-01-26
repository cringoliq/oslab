#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>
#include <pthread.h>

// Размер стека для дочернего процесса в байтах (1 МБ)
#define STACK_SIZE (1024 * 1024)

// Структура для аргументов, передаваемых в дочерний процесс
struct child_args {
    int id;     // Номер процесса
    size_t N;   // Размер задачи (в мегабайтах)
    int mode;   // Режим (0 для I/O, 1 для поиска кратчайшего пути)
};

// Генерация файла для тестирования I/O
void generateTestFile(const char* filename, size_t sizeInMB) {
    const size_t blockSize = 8 * 1024; // 8 KB
    size_t numBlocks = (sizeInMB * 1024 * 1024) / blockSize;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error creating file: " << filename << std::endl;
        return;
    }

    std::vector<char> buffer(blockSize, 'A'); // Заполняем буфер 'A'
    for (size_t i = 0; i < numBlocks; ++i) {
        file.write(buffer.data(), blockSize);
    }
    file.close();
}

// Функция для измерения пропускной способности чтения
void measureReadThroughput(const char* filename, size_t iterations) {
    const size_t blockSize = 8 * 1024; // 8 KB блок
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

// Пример поиска кратчайшего пути
void run_shortest_path_task(size_t N) {
    // Пример поиска кратчайшего пути (место для вашего алгоритма)
    std::cout << "[Short Path Task] N=" << N << ": кратчайший путь между вершинами 0 и " << N-1 << " = 123" << std::endl;
}

// Функция, которая будет выполняться дочерним процессом
static int child_func(void* arg) {
    struct child_args* cargs = (struct child_args*)arg;

    if (cargs->mode == 0) {
        // Выполняем задачу I/O
        const char* filename = "testfile.bin";
        measureReadThroughput(filename, 5);     // Измеряем пропускную способность чтения (5 итераций)
    } else {
        // Выполняем задачу поиска кратчайшего пути
        run_shortest_path_task(cargs->N);
    }

    _exit(0); // Завершаем дочерний процесс
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <N> <num_processes>" << std::endl;
        return 1;
    }

    size_t N = atol(argv[1]);      // Размер задачи (в мегабайтах)
    int num_processes = atoi(argv[2]);     // Количество процессов

    // Разделим процессы на два типа: I/O (mode=0) и поиск кратчайшего пути (mode=1)
    int io_count = num_processes / 2;
    int path_count = num_processes - io_count;

    // Массивы для хранения стеков, структур аргументов и идентификаторов процессов
    char** stacks = (char**)malloc(num_processes * sizeof(char*));
    struct child_args* cargs = (struct child_args*)malloc(num_processes * sizeof(struct child_args));
    pid_t* pids = (pid_t*)malloc(num_processes * sizeof(pid_t));

    if (!stacks || !cargs || !pids) {
        std::cerr << "Memory allocation error!" << std::endl;
        return 1;
    }

    // Создаем дочерние процессы с помощью clone()
    for (int i = 0; i < num_processes; i++) {
        stacks[i] = (char*)malloc(STACK_SIZE);
        if (!stacks[i]) {
            std::cerr << "Memory allocation error for stack!" << std::endl;
            return 1;
        }

        cargs[i].id = i;  // Номер процесса
        cargs[i].N = N;   // Размер задачи (в мегабайтах)
        cargs[i].mode = (i < io_count) ? 0 : 1;  // I/O для первых io_count процессов, поиск кратчайшего пути для остальных

        void* stackTop = stacks[i] + STACK_SIZE;  // Верхушка стека (стек растет вниз)

        pids[i] = clone(child_func, stackTop, SIGCHLD, &cargs[i]);
        if (pids[i] == -1) {
            std::cerr << "Error creating child process!" << std::endl;
            return 1;
        }
    }

    // Ожидаем завершения всех дочерних процессов
    for (int i = 0; i < num_processes; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            std::cerr << "Error waiting for child process!" << std::endl;
        } else {
            if (WIFEXITED(status)) {
                std::cout << "Child " << i << " exited with status " << WEXITSTATUS(status) << std::endl;
            } else {
                std::cout << "Child " << i << " terminated abnormally" << std::endl;
            }
        }
    }

    // Освобождаем память для стеков, аргументов и идентификаторов процессов
    for (int i = 0; i < num_processes; i++) {
        free(stacks[i]);
    }
    free(stacks);
    free(cargs);
    free(pids);

    std::cout << "All children finished." << std::endl;
    return 0;  // Завершаем программу
}

