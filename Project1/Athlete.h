#pragma once
#ifndef ATHLETE_H
#define ATHLETE_H

#include "Common.h"

class Athlete {
private:
    int id;                     // 运动员唯一编号
    std::string name;           // 运动员姓名
    Gender gender;              // 性别
    int teamId;                 // 所属参赛单位编号
    std::vector<int> eventIds;  // 参赛项目编号列表
    int maxEvents;              // 最大可参赛项目数

public:
    // 构造函数
    Athlete();
    Athlete(int id, const std::string& name, Gender gender, int teamId, int maxEvents = DEFAULT_MAX_EVENTS_PER_ATHLETE);

    // 析构函数
    ~Athlete() = default;

    // Getter 方法
    int getId() const;
    std::string getName() const;
    Gender getGender() const;
    int getTeamId() const;
    const std::vector<int>& getEventIds() const;
    int getMaxEvents() const;
    int getRegisteredEventCount() const;  // 获取已报名项目数量

    // Setter 方法
    void setId(int id);
    void setName(const std::string& name);
    void setGender(Gender gender);
    void setTeamId(int teamId);
    void setMaxEvents(int max);

    // 报名相关方法
    bool addEvent(int eventId);    // 报名项目（返回是否成功）
    bool removeEvent(int eventId); // 取消报名项目
    bool isRegistered(int eventId) const; // 是否已报名该项目
    void clearEvents();             // 清空所有报名项目

    // 文件读写方法
    bool saveToFile(std::ofstream& ofs) const;
    bool loadFromFile(std::ifstream& ifs);

    // 打印信息
    void printInfo() const;
};

#endif // ATHLETE_H