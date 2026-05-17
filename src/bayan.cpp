#include <iostream>
#include <string>
#include<set>
#include <boost/filesystem.hpp>
#include"FileSystem.h"

bool isValidDirPath(const std::string& path) {
    if (path.length() < 3) return false;

    // Первый символ - буква (A-Z или a-z)
    char first = path[0];
    if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))) {
        return false;
    }

    // Второй символ - двоеточие
    if (path[1] != ':') return false;

    // Третий символ - слеш (прямой или обратный)
    if (path[2] != '/' && path[2] != '\\') return false;

    return true;
}


void Files(boost::filesystem::path p, int flag, int currentDepth = 0, int maxDepth = 10)
{
    if (currentDepth > maxDepth) return;

    boost::system::error_code ec;

    if (!boost::filesystem::is_directory(p, ec) || ec) return;

    for (boost::filesystem::directory_iterator it(p, ec); it != boost::filesystem::directory_iterator(); ++it)
    {
        if (ec) break;

        boost::system::error_code ec2;

        if (boost::filesystem::is_regular_file(it->path(), ec2) && !ec2) {
            // Используем generic_string() для единообразных слешей
            std::cout << it->path().generic_string() << std::endl;
        }
        else if (flag != 0) {
            if (boost::filesystem::is_directory(it->path(), ec2) && !ec2) {
                std::string name = it->path().filename().string();
                // Пропускаем проблемные папки
                if (name.find('$') != std::string::npos ||
                    name.find("360") != std::string::npos) {
                    continue;
                }
                Files(it->path(), flag, currentDepth + 1, maxDepth);
            }
        }
    }
}
void printUsage() {
    std::cout << "Usage: bayan [OPTIONS]\n"
        << "Options:\n"
        << "  --scan <path>            Add directory to scan (can be used multiple times)\n"
        << "  --exclude <path>         Add directory to exclude (can be used multiple times)\n"
        << "  --level <N>              Scan depth level (0 = current dir only, default: 0)\n"
        << "  --min-size <bytes>       Minimum file size to consider (default: 1)\n"
        << "  --mask <pattern>         File name mask (can be used multiple times)\n"
        << "  --block-size <bytes>     Block size for reading (S, default: 5)\n"
        << "  --hash <algorithm>       Hash algorithm (crc32 or xor, default: xor)\n"
        << "  --help                   Show this help message\n"
        <<" Example 1: bayan --scan e:/job/test:1 --block-size 1 --min-size 1\n"
        <<" Example 2: bayan --scan E:\\scratch:inf \n"
        <<" Example 3: bayan --scan E:\\scratch:0\n"
        <<" Example 4: bayan --scan e:/job/test:1 --exclude e:/job/test/test2 --exclude e:/job/test/test3 --block-size 5 --min-size 1024\n"
        <<" Example 5: bayan --scan /home/user/docs:inf\n"
        <<" Example 6: bayan --scan e:/job/test:1 --block-size 1 --min-size 1 --hash crc32\n"
        <<" Example 7: bayan --scan E:/scratch --mask  \"*.txt\" --mask  \"*.cpp\" --mask  \"*.h\" --block-size 1"
        
        << std::endl;
}

std::string buildConfigString(int argc, char* argv[]) {
    std::string config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage();
            exit(0);
        }

        // Добавляем параметр в строку конфигурации
        config += arg;
        config += " ";

        // Добавляем значение параметра
        if (arg == "--scan" || arg == "--exclude" || arg == "--level" ||
            arg == "--min-size" || arg == "--mask" || arg == "--block-size" ||
            arg == "--hash") {
            if (i + 1 < argc) {
                config += argv[++i];
                config += " ";
            }
        }
    }

    return config;
}
/*
    Директории для сканирования	--scan	
    Директории для исключения	--exclude	
    Минимальный размер файла	--min-size	
    Размер блока S	--block-size	
    Бережное чтение с диска	Последовательное сравнение блоков	
    Чтение блока не более одного раза	Кэширование хешей	
    Дополнение нулями	std::fill	
    Ранний выход при несовпадении	return false  	
    Группировка дубликатов	printDetailedComparison	
    Вывод полных путей	files[idx].getPath()
*/

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printUsage();
        return 0;
    }

    // Собираем строку конфигурации из аргументов командной строки
    std::string configStr = buildConfigString(argc, argv);

    std::cout << "Config: " << configStr << std::endl;

    // Создаем и запускаем сканер
    FileSystem fs(configStr);

    //--scan e:/GameSystem:1 --scan e:/job:inf --exclude e:/job/vzlom
  //  FileSystem fs("--scan E:/scratch --min-size 1");
  // FileSystem fs("--scan E:/scratch --block-size 1 --min-size 1024");
  //  fs.getFileGroups().computeAllHashes(5);

    fs.printResults();
    fs.getFileGroups().printDetailedComparison();
    return 0;
}
