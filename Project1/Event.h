#pragma once
#ifndef EVENT_H
#define EVENT_H

#include "Common.h"
#include "Athlete.h"

class Event {
private:
    int id;                         // 项目唯一编号
    std::string name;               // 项目名称
    EventType type;                 // 项目类型（田赛/径赛）
    Gender genderGroup;             // 性别分组（男子/女子）
    int maxParticipants;            // 最大参赛人数
    int qualificationNum;           // 录取名次数量
    std::vector<int> scoreRules;    // 对应名次的积分规则
    std::vector<int> participantIds; // 参赛运动员编号列表

public:
    // 构造函数
    Event();
    Event(int id, const std::string& name, EventType type, Gender genderGroup, int maxParticipants = 100);

    // 析构函数
    ~Event() = default;

    // Getter 方法
    int getId() const;
    std::string getName() const;
    EventType getType() const;
    Gender getGenderGroup() const;
    int getMaxParticipants() const;
    int getQualificationNum() const;
    const std::vector<int>& getScoreRules() const;
    const std::vector<int>& getParticipantIds() const;
    int getParticipantCount() const;

    // Setter 方法
    void setId(int id);
    void setName(const std::string& name);
    void setType(EventType type);
    void setGenderGroup(Gender gender);
    void setMaxParticipants(int max);
    void setQualificationNum(int num);
    void setScoreRules(const std::vector<int>& rules);

    // 核心规则方法
    void autoSetScoreRules(); // 根据参赛人数自动设置积分规则和录取名次
    bool isEventCancelled() const; // 判断项目是否取消（人数不足4人）
    bool isBetterResult(const std::string& result1, const std::string& result2) const; // 比较两个成绩的优劣

    // 运动员报名管理
    bool addParticipant(int athleteId);
    bool removeParticipant(int athleteId);
    bool isParticipant(int athleteId) const;
    void clearParticipants();

    // 文件读写方法
    bool saveToFile(std::ofstream& ofs) const;
    bool loadFromFile(std::ifstream& ifs);

    // 打印信息
    void printInfo() const;
    void printParticipants() const;
};

#endif // EVENT_H