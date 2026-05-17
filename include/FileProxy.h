#pragma once
#include <boost/crc.hpp>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <boost/filesystem.hpp>
#include <iostream>
#include<sstream>
#include<iomanip>


struct HashRule {
    int blockSize = 5;                           // Размер блока (например 5)
    std::map<std::string, char> blockToChar;     // 
    std::string separator = "";                  // Разделитель между символами
    bool initialized = false;
    std::string algorithm = "xor";
    // Таблица символов: 0-9, A-Z, a-z (62 символа)
    static constexpr const char* HASH_SYMBOLS =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static constexpr int SYMBOL_COUNT = 62;

    // Хеширование блока "на лету" с кодированием в читаемые символы
    char hashBlockOnTheFly(const char* block, int size, uint64_t blockIndex) const {
        unsigned char numericHash = computeNumericHash(block, size, blockIndex);
        return encodeToChar(numericHash);
    }

    // Преобразование числа (0-61) в читаемый символ
    char encodeToChar(unsigned char value) const {
        return HASH_SYMBOLS[value % SYMBOL_COUNT];
    }

    // Вычисление числового хеша (0-255)
    unsigned char computeNumericHash(const char* block, int size, uint64_t blockIndex) const {
        unsigned char hash = static_cast<unsigned char>(blockIndex & 0xFF);

        for (int i = 0; i < size; i++) {
            hash ^= static_cast<unsigned char>(block[i]);
        }

        hash = (hash << 3) | (hash >> 5);
        hash ^= static_cast<unsigned char>(size);
        hash ^= static_cast<unsigned char>(blockIndex >> 8);

        return hash;
    }

    std::string computeHashForFile(const std::string& filePath,        const std::string& algorithm = "crc32") const {
        std::ifstream f(filePath, std::ios::binary);
        if (!f.is_open()) return "";

        std::string resultHash;
        std::vector<char> block(blockSize, 0);
        uint64_t blockIndex = 0;

        while (f.read(block.data(), blockSize)) {
            int bytesRead = f.gcount();
            if (bytesRead < blockSize) {
                std::fill(block.begin() + bytesRead, block.end(), 0);
            }

            std::string blockHash = hashBlock(block.data(), blockSize, blockIndex);
            resultHash += blockHash;
            blockIndex++;
        }

        return resultHash;
    }

    // "Обучить" систему: запомнить соответствие блока и символа
    void learnMapping(const std::string& blockData, char symbol) {
        blockToChar[blockData] = symbol;
    }

    // Автоматическое обучение при вычислении
    char getOrCreateBlockHash(const char* block, int size, uint64_t index) {
        std::string blockData(block, size);

        auto it = blockToChar.find(blockData);
        if (it != blockToChar.end()) {
            return it->second;
        }

        char newSymbol = hashBlockOnTheFly(block, size, index);
        blockToChar[blockData] = newSymbol;

        return newSymbol;
    }
    // CRC32 Алгоритм 
    std::string hashBlockCRC32(const char* block, int size) const {
        boost::crc_32_type result;
        result.process_bytes(block, size);
        uint32_t crc = result.checksum();

        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << crc;
        return ss.str();
    }
    // CRC32 хеширование
    uint32_t computeCRC32(const char* block, int size) const {
        boost::crc_32_type result;
        result.process_bytes(block, size);
        return result.checksum();
    }
    // Универсальный метод хэширования блока (возвращает строку)
    std::string hashBlock(const char* block, int size, uint64_t blockIndex) const {
        if (algorithm == "crc32") {
            return hashBlockCRC32(block, size);
        }
        else {  // xor по умолчанию
            return std::string(1, computeNumericHash(block, size, blockIndex));
        }
    }
    bool compareBlocks(const char* block1, const char* block2, int size,
        uint64_t blockIndex) const {
        if (algorithm == "crc32") {
            return hashBlockCRC32(block1, size) == hashBlockCRC32(block2, size);
        }
        else {
            return computeNumericHash(block1, size, blockIndex) ==
                computeNumericHash(block2, size, blockIndex);
        }
    }
};


class FileProxy {
private:
    std::string filePath;
    uintmax_t fileSize;
    std::string fileName;
    std::string hash;  // Хеш файла
    

public:
    FileProxy(const std::string& path, uintmax_t size);

    std::string getPath() const { return filePath; }
    std::string getFileName() const { return fileName; }
    uintmax_t getSize() const { return fileSize; }

    void setHash(const std::string& h) { hash = h; }
    std::string getHash() const { return hash; }
    bool hasHash() const { return !hash.empty(); }

    void print() const;

    bool operator<(const FileProxy& other) const;
};

// файлы одного размера
class SizeGroupProxy {
private:
    uintmax_t size;
    std::vector<FileProxy> files;
    HashRule hashRule;  //  Hash rule 
    std::string algorithm = "xor";
   // std::map<uintmax_t, SizeGroupProxy> groups;
    // КЭШ: для каждого файла храним вектор хешей блоков
    std::vector<std::vector<std::string>> blockHashesCache;
    std::vector<bool> cacheLoaded;
public:
    SizeGroupProxy(uintmax_t s);

    void addFile(const FileProxy& file);
    bool compareFilesCarefullyCached(int index1, int index2);
    //  хеши для всех файлов в группе
    void computeAllHashes(int blockSize = 5);
    //  хеши для ленивого сканирования
    bool compareFilesCarefully(int index1, int index2); 
    void setAlgorithm(const std::string& algo) {
        algorithm = algo;
        hashRule.algorithm = algo;
    }
    // Сравнить два файла по хешу
    bool compareFilesByHash(int index1, int index2) const;

    // Найти все дубликаты по хешам
    std::map<std::string, std::vector<int>> findDuplicatesByHash() const;
    // загрузка старых хешей
    void loadBlockHashes(int index);
    void printDetailedComparison();

    uintmax_t getSize() const { return size; }
    size_t getCount() const { return files.size(); }
    const std::vector<FileProxy>& getFiles() const { return files; }

    bool hasDuplicates() const;


    void print() const;
};


class FileGroupsProxy {
private:
    std::map<uintmax_t, SizeGroupProxy> groups;
    std::string algorithm = "xor";
public:
    void addFile(const std::string& path, uintmax_t size);

    // Вычислить хеши для всех групп
    void computeAllHashes(int blockSize = 5);
    void setAlgorithm(const std::string& algo) {  // Добавьте этот метод
        algorithm = algo;
    }
    void printAll() const;

    void printDetailedComparison();
    bool hasDuplicates() const;
    void print() const;

    void printBySize(uintmax_t minSize, uintmax_t maxSize = UINTMAX_MAX) const;

    const std::map<uintmax_t, SizeGroupProxy>& getGroups() const;
};