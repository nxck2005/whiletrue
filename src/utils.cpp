#include "utils.hpp"
#include "constants.hpp"
#include <cstdio>
#include <vector>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace Utils {

std::string formatNumber(double num) {
    if (num < 1000.0) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", num);
        return std::string(buffer);
    }

    const char* suffixes[] = {"", "K", "M", "B", "T", "Qa", "Qi", "Sx", "Sp", "Oc", "No", "Dc"};
    int suffixIndex = 0;
    double displayNum = num;

    // repeated div until num < 1000 or ran out of indexes
    while (displayNum >= 1000.0 && suffixIndex < 11) {
        displayNum /= 1000.0;
        suffixIndex++;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f%s", displayNum, suffixes[suffixIndex]);
    return std::string(buffer);
}

std::string getDataPath(const std::string& filename) {
    // Check local data first
    fs::path localPath = fs::path("./data") / filename;
    if (fs::exists(localPath)) return localPath.string();
    
    // Check global DATA_DIR
    fs::path globalPath = fs::path(DATA_DIR) / filename;
    return globalPath.string();
}

std::string getSavePath() {
    const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
    fs::path saveDir;
    if (xdgDataHome) {
        saveDir = fs::path(xdgDataHome) / "cybergrind";
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            saveDir = fs::path(home) / ".local" / "share" / "cybergrind";
        } else {
            return SAVE_FILE_NAME; // Fallback to current dir if no HOME is found
        }
    }
    
    try {
        if (!fs::exists(saveDir)) {
            fs::create_directories(saveDir);
        }
    } catch (...) {
        return SAVE_FILE_NAME; // Fallback to current dir if we can't create directory
    }
    
    return (saveDir / SAVE_FILE_NAME).string();
}

} // namespace Utils
