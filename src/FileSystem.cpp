#pragma once
#include "FileSystem.h"
#include<iostream>


ScanConfig FileSystem::parseSimplePerDirConfig(const std::string& input) {
    ScanConfig config;
    config.minFileSize = 1;
    config.blockSize = 5;  // Значение по умолчанию

    if (input.empty()) {
        config.scanDirs.emplace_back(boost::filesystem::current_path(), 0);
        return config;
    }

    std::vector<std::string> parts;
    std::istringstream iss(input);
    std::string part;

    while (iss >> part) {
        parts.push_back(part);
    }

    auto normalizePath = [](std::string path) -> std::string {
        std::replace(path.begin(), path.end(), '\\', '/');
        while (path.length() > 1 && path.back() == '/') {
            path.pop_back();
        }
        return path;
        };

    auto parseLevel = [](const std::string& str) -> int {
        if (str == "inf" || str == "INF" || str == "∞") return -1;
        try {
            return std::stoi(str);
        }
        catch (...) {
            return 0;
        }
        };

    for (size_t i = 0; i < parts.size(); i++) {
        if (parts[i] == "--scan" && i + 1 < parts.size()) {
            std::string scanArg = parts[++i];
            size_t colonPos = scanArg.rfind(':');

            if (colonPos != std::string::npos && colonPos > 1) {
                std::string path = scanArg.substr(0, colonPos);
                std::string levelStr = scanArg.substr(colonPos + 1);
                path = normalizePath(path);
                int level = parseLevel(levelStr);
                config.scanDirs.emplace_back(boost::filesystem::path(path), level);
                std::cout << "[PARSER] Added scan: '" << path << "' (level: " << level << ")" << std::endl;
            }
            else {
                std::string path = normalizePath(scanArg);
                config.scanDirs.emplace_back(boost::filesystem::path(path), 0);
                std::cout << "[PARSER] Added scan: '" << path << "' (level: 0)" << std::endl;
            }
        }
        else if (parts[i] == "--exclude" && i + 1 < parts.size()) {
            std::string path = parts[++i];
            path = normalizePath(path);
            config.excludeDirs.insert(boost::filesystem::path(path));
            std::cout << "[PARSER] Added exclude: '" << path << "'" << std::endl;
        }
        else if (parts[i] == "--hash" && i + 1 < parts.size()) {
            config.hashAlgorithm = parts[++i];
            if (config.hashAlgorithm != "xor" && config.hashAlgorithm != "crc32") {
                std::cout << "[PARSER] Unknown hash algorithm '" << config.hashAlgorithm
                    << "', using xor" << std::endl;
                config.hashAlgorithm = "xor";
            }
            else {
                std::cout << "[PARSER] Hash algorithm: " << config.hashAlgorithm << std::endl;
            }
        }
        else if (parts[i] == "--min-size" && i + 1 < parts.size()) {
            try {
                config.minFileSize = std::stoull(parts[++i]);
                std::cout << "[PARSER] Min file size: " << config.minFileSize << std::endl;
            }
            catch (...) {
                config.minFileSize = 1;
            }
        }
        else if (parts[i] == "--mask" && i + 1 < parts.size()) {
            config.nameMasks.push_back(parts[++i]);
            std::cout << "[PARSER] Added mask: " << config.nameMasks.back() << std::endl;
        }
        //  размер блока
        else if (parts[i] == "--block-size" && i + 1 < parts.size()) {
            try {
                config.blockSize = std::stoi(parts[++i]);
                if (config.blockSize < 1) config.blockSize = 1;
                if (config.blockSize > 1024) config.blockSize = 1024;  // Ограничение
                std::cout << "[PARSER] Block size: " << config.blockSize << " bytes" << std::endl;
            }
            catch (...) {
                config.blockSize = 5;
            }
        }
    }

    if (config.scanDirs.empty()) {
        config.scanDirs.emplace_back(boost::filesystem::current_path(), 0);
        std::cout << "[PARSER] No scan dirs, using current directory" << std::endl;
    }

    return config;
}