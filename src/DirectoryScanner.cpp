#include "DirectoryScanner.h"

void DirectoryScanner::scan() {
    
        std::cout << "\n=== STARTING SCAN ===" << std::endl;

        for (const auto& dirConfig : config.scanDirs) {
            std::cout << "\n--- Scanning: " << dirConfig.path.generic_string()
                << " (max level: " << (dirConfig.scanLevel == -1 ? "∞" : std::to_string(dirConfig.scanLevel))
                << ") ---" << std::endl;
            scanDirectory(dirConfig.path, dirConfig.scanLevel, 0);
        }

        std::cout << "\n=== SCAN COMPLETE ===" << std::endl;
    
}

boost::filesystem::path DirectoryScanner::normalizePath(const boost::filesystem::path& path)
{
    boost::system::error_code ec;
    boost::filesystem::path absolute = boost::filesystem::canonical(path, ec);
    if (ec) {
        absolute = boost::filesystem::absolute(path, ec);
    }
    return absolute.generic_string();
}