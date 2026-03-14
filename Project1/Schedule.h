#pragma once
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "Common.h"
#include "Event.h"
#include "Athlete.h"

class Schedule {
private:
    // 赛程项结构体
    struct ScheduleItem {
        int eventId;
        ScheduleTime time;
        std::string eventName;

        bool operator<(const ScheduleItem& other) const {
            return time < other.time;
        }
    };

    std::vector<ScheduleItem> scheduleItems;  // 完整赛程列表
    std::vector<std::string> venues;           // 可用场地列表
    int totalDays;                              // 比赛总天数
    int slotsPerMorning;                        // 上午最多可安排的项目数
    int slotsPerAfternoon;                      // 下午最多可安排的项目数

    // 内部辅助方法
    bool checkAthleteConflict(int eventId, const ScheduleTime& time, const std::vector<Athlete>& athletes, const std::vector<Event>& events) const;
    bool checkVenueConflict(const ScheduleTime& time, const std::string& venue) const;
    int getEventWeight(const Event& event) const; // 计算项目权重（用于排序优先级）

public:
    // 构造函数
    Schedule();
    Schedule(int totalDays, int slotsPerMorning, int slotsPerAfternoon);

    // 析构函数
    ~Schedule() = default;

    // 场地管理
    void addVenue(const std::string& venue);
    void clearVenues();
    const std::vector<std::string>& getVenues() const;

    // 赛程参数设置
    void setTotalDays(int days);
    void setSlotsPerMorning(int slots);
    void setSlotsPerAfternoon(int slots);

    // 核心算法：智能生成秩序册
    bool generateSchedule(const std::vector<Event>& events, const std::vector<Athlete>& athletes);

    // 赛程校验
    bool validateSchedule() const; // 校验赛程是否存在冲突

    // 赛程查询
    const std::vector<ScheduleItem>& getScheduleItems() const;
    std::vector<ScheduleItem> getScheduleByDay(int day) const;
    std::vector<ScheduleItem> getScheduleByEvent(int eventId) const;

    // 文件读写方法
    bool saveToFile(std::ofstream& ofs) const;
    bool loadFromFile(std::ifstream& ifs);

    // 打印输出
    void printFullSchedule() const;
    void printScheduleByDay(int day) const;
    void exportScheduleToTxt(const std::string& filePath) const;
};

#endif // SCHEDULE_H