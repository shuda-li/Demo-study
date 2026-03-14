#include "Common.h"

// 去除字符串首尾空格
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// 字符串转整数
bool stringToInt(const std::string& s, int& out) {
    try {
        out = std::stoi(trim(s));
        return true;
    }
    catch (...) {
        return false;
    }
}

// 字符串转浮点数
bool stringToDouble(const std::string& s, double& out) {
    try {
        out = std::stod(trim(s));
        return true;
    }
    catch (...) {
        return false;
    }
}

// 性别转字符串
std::string genderToString(Gender g) {
    return g == MALE ? "男子" : "女子";
}

// 项目类型转字符串
std::string eventTypeToString(EventType t) {
    return t == FIELD ? "田赛" : "径赛";
}