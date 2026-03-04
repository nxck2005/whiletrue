#pragma once

#include <string>

namespace Utils {
    std::string formatNumber(double num);
    std::string getDataPath(const std::string& filename);
    std::string getSavePath();
}
