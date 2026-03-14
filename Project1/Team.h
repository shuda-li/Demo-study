#pragma once
#ifndef TEAM_H
#define TEAM_H

#include "Common.h"
#include "Athlete.h"

class Team {
private:
    int id;                     // 单位唯一编号
    std::string name;           // 单位名称（学院/系/年级）
    int totalScore;             // 团体总得分
    int menScore;               // 男子团体得分
    int womenScore;             // 女子团体得分
    std::vector<int> athleteIds; // 本单位运动员编号列表

public:
    // 构造函数
    Team();
    Team(int id, const std::string& name);

    // 析构函数
    ~Team() = default;

    // Getter 方法
    int getId() const;
    std::string getName() const;
    int getTotalScore() const;
    int getMenScore() const;
    int getWomenScore() const;
    const std::vector<int>& getAthleteIds() const;

    // Setter 方法
    void setId(int id);
    void setName(const std::string& name);
    void setTotalScore(int score);
    void setMenScore(int score);
    void setWomenScore(int score);
    void addScore(int score, Gender gender); // 累计得分

    // 运动员管理方法
    bool addAthlete(int athleteId);
    bool removeAthlete(int athleteId);
    bool hasAthlete(int athleteId) const;
    void clearAthletes();

    // 分数重置
    void resetScore();

    // 文件读写方法
    bool saveToFile(std::ofstream& ofs) const;
    bool loadFromFile(std::ifstream& ifs);

    // 打印信息
    void printInfo() const;
    void printScoreRank() const;
};

#endif // TEAM_H