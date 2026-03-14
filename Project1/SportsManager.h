#pragma once
#ifndef SPORTS_MANAGER_H
#define SPORTS_MANAGER_H

#include "Common.h"
#include "Team.h"
#include "Event.h"
#include "Athlete.h"
#include "Result.h"
#include "Schedule.h"

class SportsManager {
private:
    // 核心数据容器
    std::vector<Team> teams;
    std::vector<Event> events;
    std::vector<Athlete> athletes;
    std::vector<Result> results;
    Schedule schedule;

    // 系统配置参数
    int maxEventsPerAthlete;
    std::string dataDir; // 数据文件存储目录

    // 内部辅助方法
    int getNextTeamId() const;
    int getNextEventId() const;
    int getNextAthleteId() const;
    bool isDirExist(const std::string& dirPath) const;
    bool createDir(const std::string& dirPath) const;

    // 数据加载与保存
    bool loadAllData();
    bool saveAllData() const;

    // 成绩统计核心方法
    void calculateAllRankAndScore();
    void updateTeamTotalScore();

public:
    // 构造函数与析构函数
    SportsManager(const std::string& dataDir = "./data");
    ~SportsManager() = default;

    // 系统初始化
    bool initSystem();
    void resetSystem();

    // 系统设置相关
    void setMaxEventsPerAthlete(int max);
    int getMaxEventsPerAthlete() const;
    void setScheduleConfig(int totalDays, int slotsPerMorning, int slotsPerAfternoon);
    void addScheduleVenue(const std::string& venue);

    // 参赛单位管理
    bool addTeam(const std::string& name);
    bool removeTeam(int teamId);
    bool modifyTeamName(int teamId, const std::string& newName);
    const Team* getTeamById(int teamId) const;
    Team* getTeamById(int teamId);
    const std::vector<Team>& getAllTeams() const;
    void printAllTeams() const;

    // 比赛项目管理
    bool addEvent(const std::string& name, EventType type, Gender genderGroup, int maxParticipants);
    bool removeEvent(int eventId);
    bool modifyEvent(int eventId, const std::string& name, EventType type, Gender genderGroup);
    const Event* getEventById(int eventId) const;
    Event* getEventById(int eventId);
    const std::vector<Event>& getAllEvents() const;
    void printAllEvents() const;
    void checkAndCancelInvalidEvents(); // 检查并取消人数不足的项目

    // 运动员管理
    bool addAthlete(const std::string& name, Gender gender, int teamId);
    bool removeAthlete(int athleteId);
    bool modifyAthlete(int athleteId, const std::string& name, Gender gender, int teamId);
    const Athlete* getAthleteById(int athleteId) const;
    Athlete* getAthleteById(int athleteId);
    const std::vector<Athlete>& getAllAthletes() const;
    void printAthletesByTeam(int teamId) const;
    void printAllAthletes() const;

    // 报名管理
    bool athleteRegisterEvent(int athleteId, int eventId);
    bool athleteCancelRegister(int athleteId, int eventId);
    void printAthleteRegisterInfo(int athleteId) const;
    void printEventRegisterInfo(int eventId) const;

    // 秩序册管理
    bool generateSchedule();
    void printFullSchedule() const;
    void exportSchedule(const std::string& filePath) const;

    // 成绩管理
    bool addResult(int eventId, int athleteId, const std::string& result);
    bool modifyResult(int eventId, int athleteId, const std::string& newResult);
    bool removeResult(int eventId, int athleteId);
    void calculateEventResult(int eventId); // 计算单个项目的名次和得分
    const Result* getResult(int eventId, int athleteId) const;
    std::vector<Result> getResultsByEvent(int eventId) const;
    std::vector<Result> getResultsByAthlete(int athleteId) const;
    std::vector<Result> getResultsByTeam(int teamId) const;

    // 成绩查询与统计
    void printEventResult(int eventId) const;
    void printAthleteResult(int athleteId) const;
    void printTeamResult(int teamId) const;
    void printTeamRank() const;
    void printGenderGroupRank(Gender gender) const;

    // 数据备份与恢复
    bool backupData(const std::string& backupDir) const;
    bool restoreData(const std::string& backupDir);
    bool exportDataToTxt(const std::string& exportDir) const;

    // 系统主菜单
    void showMainMenu();
    void run();
};

#endif // SPORTS_MANAGER_H