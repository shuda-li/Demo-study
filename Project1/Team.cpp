#include "Team.h"

// 构造函数
Team::Team() : id(0), totalScore(0), menScore(0), womenScore(0) {}

Team::Team(int id, const std::string& name)
    : id(id), name(name), totalScore(0), menScore(0), womenScore(0) {}

// Getter 实现
int Team::getId() const { return id; }
std::string Team::getName() const { return name; }
int Team::getTotalScore() const { return totalScore; }
int Team::getMenScore() const { return menScore; }
int Team::getWomenScore() const { return womenScore; }
const std::vector<int>& Team::getAthleteIds() const { return athleteIds; }

// Setter 实现
void Team::setId(int id) { this->id = id; }
void Team::setName(const std::string& name) { this->name = name; }
void Team::setTotalScore(int score) { totalScore = score; }
void Team::setMenScore(int score) { menScore = score; }
void Team::setWomenScore(int score) { womenScore = score; }

// 累计得分
void Team::addScore(int score, Gender gender) {
    totalScore += score;
    if (gender == MALE) {
        menScore += score;
    }
    else {
        womenScore += score;
    }
}

// 添加运动员
bool Team::addAthlete(int athleteId) {
    if (hasAthlete(athleteId)) return false;
    athleteIds.push_back(athleteId);
    return true;
}

// 移除运动员
bool Team::removeAthlete(int athleteId) {
    auto it = std::find(athleteIds.begin(), athleteIds.end(), athleteId);
    if (it == athleteIds.end()) return false;
    athleteIds.erase(it);
    return true;
}

// 检查是否包含运动员
bool Team::hasAthlete(int athleteId) const {
    return std::find(athleteIds.begin(), athleteIds.end(), athleteId) != athleteIds.end();
}

// 清空运动员列表
void Team::clearAthletes() {
    athleteIds.clear();
}

// 重置分数
void Team::resetScore() {
    totalScore = 0;
    menScore = 0;
    womenScore = 0;
}

// 保存到文件
bool Team::saveToFile(std::ofstream& ofs) const {
    if (!ofs.is_open()) return false;
    ofs << id << "|" << name << "|" << totalScore << "|" << menScore << "|" << womenScore;
    for (int aid : athleteIds) ofs << "|" << aid;
    ofs << std::endl;
    return ofs.good();
}

// 从文件加载
bool Team::loadFromFile(std::ifstream& ifs) {
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

    int tempId, tempTotal, tempMen, tempWomen;
    if (!stringToInt(items[0], tempId)) return false;
    if (!stringToInt(items[2], tempTotal)) return false;
    if (!stringToInt(items[3], tempMen)) return false;
    if (!stringToInt(items[4], tempWomen)) return false;

    id = tempId;
    name = items[1];
    totalScore = tempTotal;
    menScore = tempMen;
    womenScore = tempWomen;
    athleteIds.clear();

    for (size_t i = 5; i < items.size(); i++) {
        int aid;
        if (stringToInt(items[i], aid)) {
            athleteIds.push_back(aid);
        }
    }

    return true;
}

// 打印单位信息
void Team::printInfo() const {
    std::cout << std::left << std::setw(8) << id
        << std::setw(20) << name
        << std::setw(10) << totalScore
        << std::setw(10) << menScore
        << std::setw(10) << womenScore
        << std::setw(10) << athleteIds.size()
        << std::endl;
}

// 打印排名信息
void Team::printScoreRank() const {
    std::cout << std::left << std::setw(20) << name
        << std::setw(10) << totalScore
        << std::setw(10) << menScore
        << std::setw(10) << womenScore
        << std::endl;
}