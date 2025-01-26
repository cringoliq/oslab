# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(oslab1)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции компиляции
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g3")

# Пути для включения заголовочных файлов
include_directories(include)

# Основной исполняемый файл
add_executable(main src/main.cpp src/shell.cpp
    shell.h)

# Бенчмарки: отдельный исполняемый файл
add_executable(benchmark benchmarks/benchmark.cpp src/shell.cpp
    benchmarks/short_path.h benchmarks/short_path.cpp
    benchmarks/io_thpt_read.h benchmarks/io_thpt_read.cpp)

# Связанные библиотеки (при необходимости)
# target_link_libraries(main ...)
# target_link_libraries(benchmark ...)
