#pragma once
#ifndef COMMON_H
#define COMMON_H
#ifdef _WIN32
#include <direct.h>  // Windows 目录操作头文件
#endif
#include <filesystem>  // C++17 标准库
namespace fs = std::filesystem;  // 命名空间别名，方便使用

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>
#include <ctime>

// 系统全局常量定义
const int DEFAULT_MAX_EVENTS_PER_ATHLETE = 3;  // 默认运动员最多参赛项目数
const int MIN_PARTICIPANTS_FOR_EVENT = 4;       // 项目最低参赛人数（不足则取消）
const int THRESHOLD_PARTICIPANTS = 6;           // 录取名次分界人数

// 项目类型枚举
enum EventType {
    FIELD = 'T',    // 田赛
    TRACK = 'R'     // 径赛
};

// 性别枚举
enum Gender {
    MALE = 'M',      // 男子
    FEMALE = 'F'     // 女子
};

// 赛程时间结构体
struct ScheduleTime {
    int day;                // 比赛天数（第几天）
    bool isMorning;         // 上午/下午
    int timeSlot;           // 时间段序号（每个项目占用1个slot）
    std::string venue;      // 比赛场地

    // 重载比较运算符，用于时间排序
    bool operator<(const ScheduleTime& other) const {
        if (day != other.day) return day < other.day;
        if (isMorning != other.isMorning) return isMorning > other.isMorning;
        return timeSlot < other.timeSlot;
    }

    // 判断两个时间段是否冲突
    bool isConflict(const ScheduleTime& other) const {
        return day == other.day && isMorning == other.isMorning && timeSlot == other.timeSlot;
    }
};

// 工具函数声明
std::string trim(const std::string& s);
bool stringToInt(const std::string& s, int& out);
bool stringToDouble(const std::string& s, double& out);
std::string genderToString(Gender g);
std::string eventTypeToString(EventType t);

#endif // COMMON_H