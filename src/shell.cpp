#include "../shell.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Разбивает строку на аргументы команды
std::vector<std::string> splitCommand(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string token;
    while (iss >> token) {
        args.push_back(token);
    }
    return args;
}

// Запуск оболочки
void runShell() {
    std::string input;

    while (true) {
        std::cout << "shell> ";
        std::getline(std::cin, input);

        // Завершение работы оболочки
        if (input == "exit") {
            std::cout << "Exiting shell..." << std::endl;
            break;
        }

        // Разбить строку на команду и аргументы
        std::vector<std::string> args = splitCommand(input);
        if (args.empty()) continue;

        // Преобразовать std::vector<std::string> в массив C-строк
        std::vector<char*> c_args;
        for (auto& arg : args) {
            c_args.push_back(&arg[0]);
        }
        c_args.push_back(nullptr);

        // Фиксация времени запуска
        auto start = std::chrono::high_resolution_clock::now();

        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс: выполнить команду
            execvp(c_args[0], c_args.data());
            // Если execvp вернулся, значит команда не была выполнена
            std::cerr << "Error: command not found: " << args[0] << std::endl;
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Родительский процесс: ожидать завершения команды
            int status;
            waitpid(pid, &status, 0);

            // Фиксация времени завершения
            auto end = std::chrono::high_resolution_clock::now();

            // Вычисление времени выполнения
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Command executed in " << elapsed.count() << " ms" << std::endl;
        } else {
            // Ошибка fork()
            std::cerr << "Error: failed to fork process" << std::endl;
        }
    }
}
