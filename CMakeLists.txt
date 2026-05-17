cmake_minimum_required(VERSION 3.10)
project(bayan)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Boost
find_package(Boost REQUIRED COMPONENTS filesystem system)

# Включаемые директории
include_directories(include)

# Исполняемый файл bayan
add_executable(bayan
    src/bayan.cpp
    src/FileSystem.cpp
    src/FileProxy.cpp
    src/DirectoryScanner.cpp
)

# Линкуем с Boost
target_link_libraries(bayan PRIVATE Boost::filesystem Boost::system)

# Опционально: добавить отладочные флаги
target_compile_options(bayan PRIVATE -Wall -Wextra -O2)

# Тесты (опционально)
enable_testing()
add_executable(test_scan
    test/test_scan.cpp
    src/FileSystem.cpp
    src/FileProxy.cpp
    src/DirectoryScanner.cpp
)

target_link_libraries(test_scan PRIVATE Boost::filesystem Boost::system)
target_compile_options(test_scan PRIVATE -Wall -Wextra -g)
