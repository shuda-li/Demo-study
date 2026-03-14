#include "Result.h"

// 构造函数
Result::Result() : eventId(0), athleteId(0), rank(0), score(0) {}

Result::Result(int eventId, int athleteId, const std::string& result, int rank, int score)
    : eventId(eventId), athleteId(athleteId), result(result), rank(rank), score(score) {}

// Getter 实现
int Result::getEventId() const { return eventId; }
int Result::getAthleteId() const { return athleteId; }
std::string Result::getResult() const { return result; }
int Result::getRank() const { return rank; }
int Result::getScore() const { return score; }

// Setter 实现
void Result::setEventId(int id) { eventId = id; }
void Result::setAthleteId(int id) { athleteId = id; }
void Result::setResult(const std::string& result) { this->result = result; }
void Result::setRank(int rank) { this->rank = rank; }
void Result::setScore(int score) { this->score = score; }

// 成绩转数值
double Result::getResultValue() const {
    double val;
    if (stringToDouble(result, val)) {
        return val;
    }
    return 0.0;
}

// 保存到文件
bool Result::saveToFile(std::ofstream& ofs) const {
    if (!ofs.is_open()) return false;
    ofs << eventId << "|" << athleteId << "|" << result << "|" << rank << "|" << score << std::endl;
    return ofs.good();
}

// 从文件加载
bool Result::loadFromFile(std::ifstream& ifs) {
    if (!ifs.is_open()) return false;
    std::string line;
    if (!std::getline(ifs, line)) return false;
    line = trim(line);
    if (line.empty()) return false;

    std::stringstream ss(line);
    std::string item;
    std::vector<std::string> items;

    while (std::getline(ss, item, '|')) {
        items.push_back(trim(item));
    }

    if (items.size() < 5) return false;

    int tempEventId, tempAthleteId, tempRank, tempScore;
    if (!stringToInt(items[0], tempEventId)) return false;
    if (!stringToInt(items[1], tempAthleteId)) return false;
    if (!stringToInt(items[3], tempRank)) return false;
    if (!stringToInt(items[4], tempScore)) return false;

    eventId = tempEventId;
    athleteId = tempAthleteId;
    result = items[2];
    rank = tempRank;
    score = tempScore;

    return true;
}

// 打印成绩信息
void Result::printInfo(const std::string& athleteName, const std::string& eventName) const {
    std::cout << std::left << std::setw(8) << rank
        << std::setw(12) << athleteName
        << std::setw(16) << eventName
        << std::setw(12) << result
        << std::setw(8) << score
        << std::endl;
}