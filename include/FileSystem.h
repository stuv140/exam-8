#pragma once
#include"DirectoryScanner.h"
#include <string>
#include <vector>
#include<sstream>
#include"FileProxy.h"
#include <regex>


class FileSystem
{
public:
	FileSystem(const std::string& configStr)
		: scanner(parseSimplePerDirConfig(configStr)) {
        scanner.scan();
                 // Вычисляем хеши с указанным размером блока
        scanner.getFileGroups().computeAllHashes(scanner.getBlockSize());

	}
	
    void scan() {
        scanner.scan();
    }

    void printResults() const {
        scanner.printResults();
    }

    void printResultsBySize(uintmax_t minSize, uintmax_t maxSize = UINTMAX_MAX) const {
        scanner.printResultsBySize(minSize, maxSize);
    }

    FileGroupsProxy& getFileGroups() {
        return scanner.getFileGroups();
    }
private:
	ScanConfig parseSimplePerDirConfig(const std::string& input);
	DirectoryScanner scanner;
	
};
