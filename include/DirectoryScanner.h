#pragma once
#include "Config.h"
#include "FileProxy.h"
#include <boost/filesystem.hpp>
#include<algorithm>
#include<iostream>

class DirectoryScanner
{
public:
    DirectoryScanner(const ScanConfig& cfg) : config(cfg) {
        for (const auto& dir : config.excludeDirs) {
            normalizedExcludeDirs.insert(normalizePath(dir));
        }
        fileGroups.setAlgorithm(config.hashAlgorithm);  // Устанавливаем алгоритм
    }
    int getBlockSize() const { return config.blockSize; }
    void scan(); 
    
    void printResults() const {
        fileGroups.printAll();
    }

    void printResultsBySize(uintmax_t minSize, uintmax_t maxSize = UINTMAX_MAX) const {
        fileGroups.printBySize(minSize, maxSize);
    }

    FileGroupsProxy& getFileGroups() {
        return fileGroups;
    }
private:
    ScanConfig config;
    FileGroupsProxy fileGroups;
    std::set<boost::filesystem::path> normalizedExcludeDirs;
    // Нормализация пути для сравнения
    boost::filesystem::path normalizePath(const boost::filesystem::path& path);
    // Проверка, нужно ли исключить директорию
    bool shouldExclude(const boost::filesystem::path& dirPath) {
        boost::system::error_code ec;
        boost::filesystem::path normalized = normalizePath(dirPath);

        for (const auto& excludeDir : normalizedExcludeDirs) {
            std::string normStr = normalized.generic_string();
            std::string exclStr = excludeDir.generic_string();

            if (normStr == exclStr || normStr.find(exclStr) == 0) {
                return true;
            }
        }
        return false;
    }
    // Рекурсивное сканирование с индивидуальным уровнем
    // Рекурсивное сканирование с индивидуальным уровнем
    void scanDirectory(const boost::filesystem::path& dir,
        int maxLevel,
        int currentLevel = 0) {
        // Проверка уровня
        if (maxLevel >= 0 && currentLevel > maxLevel) {
            return;
        }

        // Проверка исключений
        if (shouldExclude(dir)) {
            std::cout << "[EXCLUDED] " << dir.generic_string() << std::endl;
            return;
        }

        boost::system::error_code ec;

        if (!boost::filesystem::exists(dir, ec) || ec) {
            std::cout << "[ERROR] Path not found: " << dir.generic_string() << std::endl;
            return;
        }

        if (!boost::filesystem::is_directory(dir, ec) || ec) {
            std::cout << "[ERROR] Not a directory: " << dir.generic_string() << std::endl;
            return;
        }

        // Отображение уровня для отладки
        std::string indent(currentLevel * 2, ' ');
        std::cout << indent << "[SCAN] " << dir.generic_string()
            << " (level " << currentLevel << "/"
            << (maxLevel == -1 ? "∞" : std::to_string(maxLevel)) << ")" << std::endl;

        // Открываем директорию
        boost::filesystem::directory_iterator it(dir, ec);
        if (ec) {
            std::cout << indent << "[ERROR] Cannot open directory: " << ec.message() << std::endl;
            return;
        }

        boost::filesystem::directory_iterator end;

        for (; it != end; ++it) {
            // Сбрасываем error_code для каждой итерации
            boost::system::error_code ec2;

            // Получаем путь к элементу
            const auto& itemPath = it->path();

            // Проверяем, является ли файлом
            if (boost::filesystem::is_regular_file(itemPath, ec2) && !ec2) {
                uintmax_t size = boost::filesystem::file_size(itemPath, ec2);
                // ПРОВЕРКА МАСКИ И МИНИМАЛЬНОГО РАЗМЕРА
                if (!ec2 && size >= config.minFileSize && config.matchesMask(itemPath.filename().string())) {
                    fileGroups.addFile(itemPath.generic_string(), size);
                }
            }
            // Проверяем, является ли директорией
            else if (boost::filesystem::is_directory(itemPath, ec2) && !ec2) {
                // Проверяем, не является ли символической ссылкой (чтобы избежать циклов)
                if (boost::filesystem::is_symlink(itemPath, ec2)) {
                    std::cout << indent << "  [SKIP SYMLINK] " << itemPath.filename().string() << std::endl;
                    continue;
                }

                // Рекурсивный обход поддиректории
                scanDirectory(itemPath, maxLevel, currentLevel + 1);
            }
        }
    }


};