#pragma once
#ifndef RESULT_H
#define RESULT_H

#include "Common.h"

class Result {
private:
    int eventId;        // 所属项目编号
    int athleteId;      // 运动员编号
    std::string result; // 比赛成绩原始字符串
    int rank;           // 名次
    int score;          // 对应得分

public:
    // 构造函数
    Result();
    Result(int eventId, int athleteId, const std::string& result, int rank = 0, int score = 0);

    // 析构函数
    ~Result() = default;

    // Getter 方法
    int getEventId() const;
    int getAthleteId() const;
    std::string getResult() const;
    int getRank() const;
    int getScore() const;

    // Setter 方法
    void setEventId(int id);
    void setAthleteId(int id);
    void setResult(const std::string& result);
    void setRank(int rank);
    void setScore(int score);

    // 成绩数值转换
    double getResultValue() const; // 将成绩字符串转为数值用于排序

    // 文件读写方法
    bool saveToFile(std::ofstream& ofs) const;
    bool loadFromFile(std::ifstream& ifs);

    // 打印信息
    void printInfo(const std::string& athleteName, const std::string& eventName) const;
};

#endif // RESULT_H