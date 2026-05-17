#include"FileProxy.h"


FileProxy::FileProxy(const std::string& path, uintmax_t size)
    : filePath(path), fileSize(size) {
    boost::filesystem::path p(path);
    fileName = p.filename().string();
}

void FileProxy::print() const {
    std::cout << "    " << filePath << " (" << fileSize << " bytes)" << std::endl;
}

bool FileProxy::operator<(const FileProxy& other) const {
    return fileSize < other.fileSize;
}


SizeGroupProxy::SizeGroupProxy(uintmax_t s) : size(s) {
    hashRule.initialized = true;
}



void SizeGroupProxy::computeAllHashes(int blockSize) {
    hashRule.blockSize = blockSize;
    for (auto& file : files) {
        std::string hash = hashRule.computeHashForFile(file.getPath());
        file.setHash(hash);
    }
}

bool SizeGroupProxy::compareFilesByHash(int index1, int index2) const {
    if (index1 >= static_cast<int>(files.size()) ||
        index2 >= static_cast<int>(files.size())) {
        return false;
    }

    const auto& file1 = files[index1];
    const auto& file2 = files[index2];

    if (!file1.hasHash() || !file2.hasHash()) {
        return false;
    }

    return file1.getHash() == file2.getHash();
}

std::map<std::string, std::vector<int>> SizeGroupProxy::findDuplicatesByHash() const {
    std::map<std::string, std::vector<int>> hashGroups;

    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].hasHash()) {
            hashGroups[files[i].getHash()].push_back(i);
        }
    }

    return hashGroups;
}

bool SizeGroupProxy::compareFilesCarefully(int index1, int index2) {
    // Загружаем хеши (если еще не загружены)
    loadBlockHashes(index1);
    loadBlockHashes(index2);

    const auto& hashes1 = blockHashesCache[index1];
    const auto& hashes2 = blockHashesCache[index2];

    // Сравниваем хеши блоков
    size_t minSize = std::min(hashes1.size(), hashes2.size());
    for (size_t i = 0; i < minSize; i++) {
        if (hashes1[i] != hashes2[i]) {
            return false;
        }
    }

    // Если количество блоков одинаковое - файлы идентичны
    return hashes1.size() == hashes2.size();

   // return true;
}


bool SizeGroupProxy::hasDuplicates() const {
    if (files.size() < 2) return false;

    if (!files.empty() && !files[0].hasHash()) {
        return files.size() >= 2;
    }

    auto duplicates = findDuplicatesByHash();
    for (const auto& pair : duplicates) {   
        if (pair.second.size() > 1) return true;
    }
    return false;
}
void SizeGroupProxy::addFile(const FileProxy& file) {
    files.push_back(file);
}

void SizeGroupProxy::print() const {
    if (files.size() >= 2) {
        std::cout << "\n  [" << size << " bytes] - " << files.size() << " files:" << std::endl;
        for (const auto& file : files) {
            file.print();
        }

        if (!files.empty() && files[0].hasHash()) {
            auto duplicates = findDuplicatesByHash();
            std::cout << "    Hash analysis: " << duplicates.size() << " unique hash groups" << std::endl;

            for (const auto& pair : duplicates) {  // ← исправлено
                if (pair.second.size() > 1) {
                    std::cout << "      " << pair.second.size() << " files share same hash" << std::endl;
                }
            }
        }
    }
}
void SizeGroupProxy::loadBlockHashes(int index)
{
    
        if (cacheLoaded[index]) return;

        const auto& file = files[index];
        std::ifstream f(file.getPath(), std::ios::binary);
        if (!f.is_open()) return;

        std::vector<char> block(hashRule.blockSize, 0);
        std::vector<std::string> hashes;
        uint64_t blockIndex = 0;

        while (f.read(block.data(), hashRule.blockSize)) {
            int bytesRead = f.gcount();
            if (bytesRead < hashRule.blockSize) {
                std::fill(block.begin() + bytesRead, block.end(), 0);
            }
            std::string blockHash = hashRule.hashBlock(block.data(), hashRule.blockSize, blockIndex);
            hashes.push_back(blockHash);
            blockIndex++;
        }

        blockHashesCache[index] = hashes;
        cacheLoaded[index] = true;
    
}
void SizeGroupProxy::printDetailedComparison() {
    if (files.size() < 2) return;

    // Инициализируем кэш
    blockHashesCache.resize(files.size());
    cacheLoaded.resize(files.size(), false);

    std::cout << "\n  Detailed comparison for size " << size << " bytes:" << std::endl;
    std::cout << "  Algorithm: " << algorithm << std::endl;

    std::vector<bool> processed(files.size(), false);

    for (size_t i = 0; i < files.size(); i++) {
        if (processed[i]) continue;

        // Загружаем хеши для файла i (один раз)
        loadBlockHashes(i);

        std::vector<int> duplicates;
        duplicates.push_back(i);

        for (size_t j = i + 1; j < files.size(); j++) {
            if (processed[j]) continue;

            // Загружаем хеши для файла j (один раз)
            loadBlockHashes(j);

            if (compareFilesCarefullyCached(i, j)) {
                duplicates.push_back(j);
                processed[j] = true;
            }
        }

        if (duplicates.size() > 1) {
            std::cout << "    Group of " << duplicates.size() << " identical files:" << std::endl;
            for (int idx : duplicates) {
                std::cout << "      - " << files[idx].getFileName() << std::endl;
            }
        }

        processed[i] = true;
    }

}
// Сравнение с использованием кэша
bool SizeGroupProxy::compareFilesCarefullyCached(int index1, int index2) {
    // Загружаем хеши (если еще не загружены)
    loadBlockHashes(index1);
    loadBlockHashes(index2);

    const auto& hashes1 = blockHashesCache[index1];
    const auto& hashes2 = blockHashesCache[index2];

    // Сравниваем хеши блоков
    size_t minSize = std::min(hashes1.size(), hashes2.size());
    for (size_t i = 0; i < minSize; i++) {
        if (hashes1[i] != hashes2[i]) {
            return false;
        }
    }

    // Если количество блоков одинаковое - файлы идентичны
    return hashes1.size() == hashes2.size();
}

void FileGroupsProxy::addFile(const std::string& path, uintmax_t size) {
    auto it = groups.find(size);
    if (it != groups.end()) {
        it->second.addFile(FileProxy(path, size));
    }
    else {
        SizeGroupProxy newGroup(size);
        newGroup.addFile(FileProxy(path, size));
        groups.emplace(size, newGroup);
    }
}

void FileGroupsProxy::computeAllHashes(int blockSize) {
    for (auto& pair : groups) {
        pair.second.computeAllHashes(blockSize);
    }
}

void FileGroupsProxy::printAll() const {
    std::cout << "\n=== FILES GROUPED BY SIZE ===" << std::endl;
    bool found = false;
    for (const auto& pair : groups) {   
        if (pair.second.hasDuplicates()) {
            pair.second.print();
            found = true;
        }
    }
    if (!found) {
        std::cout << "No duplicate file sizes found." << std::endl;
    }
}

void FileGroupsProxy::printDetailedComparison() {
    std::cout << "\n=== DETAILED COMPARISON ===" << std::endl;
    for (auto& pair : groups) {
        if (pair.second.getCount() >= 2) {
            pair.second.printDetailedComparison();
        }
    }
}

void FileGroupsProxy::printBySize(uintmax_t minSize, uintmax_t maxSize) const {
    std::cout << "\n=== FILES SIZE " << minSize << " - "
        << (maxSize == UINTMAX_MAX ? "INF" : std::to_string(maxSize))
        << " bytes ===" << std::endl;

    for (const auto& pair : groups) {   
        if (pair.first >= minSize && pair.first <= maxSize && pair.second.hasDuplicates()) {
            pair.second.print();
        }
    }
}

const std::map<uintmax_t, SizeGroupProxy>& FileGroupsProxy::getGroups() const {
    return groups;
}