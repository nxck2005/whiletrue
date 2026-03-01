#include "utils.hpp"
#include <cstdio>
#include <vector>

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

} // namespace Utils
