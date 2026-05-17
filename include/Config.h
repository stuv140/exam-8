#pragma once
#include<iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <regex>
#include <boost/filesystem.hpp>

struct DirectoryConfig {
    boost::filesystem::path path;
    int scanLevel;  // -1 - без ограничений, 0 - только текущая, 1,2,3...

    DirectoryConfig(const boost::filesystem::path& p, int level = 0)
        : path(p), scanLevel(level) {
    }
};


struct ScanConfig
{
    // Директории для сканирования с индивидуальным уровнем
    std::vector<DirectoryConfig> scanDirs;

    // Директории для исключения
    std::set<boost::filesystem::path> excludeDirs;

    // Глобальный минимальный размер файла (по умолчанию > 1 байта)
    uintmax_t minFileSize = 1;
    int blockSize = 5;  // размер блока для хеширования
    void print() const {
        std::cout << "\n=== SCAN CONFIGURATION ===" << std::endl;
        std::cout << "Scan directories:" << std::endl;
        for (const auto& dir : scanDirs) {
            std::cout << "  - " << dir.path.generic_string()
                << " (level: " << (dir.scanLevel == -1 ? "∞" : std::to_string(dir.scanLevel))
                << ")" << std::endl;
        }

        std::cout << "Exclude directories:" << std::endl;
        for (const auto& dir : excludeDirs) {
            std::cout << "  - " << dir.generic_string() << std::endl;
        }

        std::cout << "Min file size: " << minFileSize << " bytes" << std::endl;
        std::cout << "Block size: " << blockSize << " bytes" << std::endl;
        std::cout << "Hash algorithm: " << hashAlgorithm << std::endl;
        std::cout << "=========================" << std::endl;
        if (!nameMasks.empty()) {
            std::cout << "Name masks: ";
            for (const auto& m : nameMasks) std::cout << m << " ";
            std::cout << std::endl;
        }
    
    }
    // Метод для проверки соответствия маске
    bool matchesMask(const std::string& filename) const {
        if (nameMasks.empty()) return true;  // Если масок нет - все файлы подходят

        std::string name = filename;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        for (const auto& mask : nameMasks) {
            std::string m = mask;
            std::transform(m.begin(), m.end(), m.begin(), ::tolower);

            // Простое сравнение с маской (поддерживает *)
            if (m.find('*') == std::string::npos) {
                if (name == m) return true;
            }
            else {
                // Преобразуем маску в регулярное выражение
                std::string pattern = "^" +
                    std::regex_replace(m, std::regex("\\*"), ".*") + "$";
                std::regex re(pattern);
                if (std::regex_match(name, re)) return true;
            }
        }
        return false;
    }

    std::string hashAlgorithm = "xor";  // "crc32" или "xor"
    std::vector<std::string> nameMasks;
};