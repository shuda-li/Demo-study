#include "Event.h"

// 构造函数
Event::Event() : id(0), type(TRACK), genderGroup(MALE), maxParticipants(100), qualificationNum(0) {}

Event::Event(int id, const std::string& name, EventType type, Gender genderGroup, int maxParticipants)
    : id(id), name(name), type(type), genderGroup(genderGroup), maxParticipants(maxParticipants), qualificationNum(0) {}

// Getter 实现
int Event::getId() const { return id; }
std::string Event::getName() const { return name; }
EventType Event::getType() const { return type; }
Gender Event::getGenderGroup() const { return genderGroup; }
int Event::getMaxParticipants() const { return maxParticipants; }
int Event::getQualificationNum() const { return qualificationNum; }
const std::vector<int>& Event::getScoreRules() const { return scoreRules; }
const std::vector<int>& Event::getParticipantIds() const { return participantIds; }
int Event::getParticipantCount() const { return participantIds.size(); }

// Setter 实现
void Event::setId(int id) { this->id = id; }
void Event::setName(const std::string& name) { this->name = name; }
void Event::setType(EventType type) { this->type = type; }
void Event::setGenderGroup(Gender gender) { genderGroup = gender; }
void Event::setMaxParticipants(int max) { maxParticipants = max; }
void Event::setQualificationNum(int num) { qualificationNum = num; }
void Event::setScoreRules(const std::vector<int>& rules) { scoreRules = rules; }

// 自动设置积分规则和录取名次
void Event::autoSetScoreRules() {
    int participantNum = participantIds.size();
    scoreRules.clear();

    if (participantNum > THRESHOLD_PARTICIPANTS) {
        qualificationNum = 5;
        scoreRules = { 7, 5, 3, 2, 1 };
    }
    else if (participantNum >= MIN_PARTICIPANTS_FOR_EVENT) {
        qualificationNum = 3;
        scoreRules = { 5, 3, 2 };
    }
    else {
        qualificationNum = 0;
    }
}

// 判断项目是否取消
bool Event::isEventCancelled() const {
    return participantIds.size() < MIN_PARTICIPANTS_FOR_EVENT;
}

// 比较成绩优劣（核心：区分田赛和径赛）
bool Event::isBetterResult(const std::string& result1, const std::string& result2) const {
    double val1, val2;
    if (!stringToDouble(result1, val1) || !stringToDouble(result2, val2)) {
        return false;
    }

    // 径赛：数值越小成绩越好（时间越短）
    // 田赛：数值越大成绩越好（高度/远度越高）
    if (type == TRACK) {
        return val1 < val2;
    }
    else {
        return val1 > val2;
    }
}

// 添加参赛运动员
bool Event::addParticipant(int athleteId) {
    if (isParticipant(athleteId)) return false;
    if (participantIds.size() >= maxParticipants) return false;
    participantIds.push_back(athleteId);
    autoSetScoreRules(); // 重新计算积分规则
    return true;
}

// 移除参赛运动员
bool Event::removeParticipant(int athleteId) {
    auto it = std::find(participantIds.begin(), participantIds.end(), athleteId);
    if (it == participantIds.end()) return false;
    participantIds.erase(it);
    autoSetScoreRules(); // 重新计算积分规则
    return true;
}

// 检查是否为参赛运动员
bool Event::isParticipant(int athleteId) const {
    return std::find(participantIds.begin(), participantIds.end(), athleteId) != participantIds.end();
}

// 清空参赛列表
void Event::clearParticipants() {
    participantIds.clear();
    autoSetScoreRules();
}

// 保存到文件
bool Event::saveToFile(std::ofstream& ofs) const {
    if (!ofs.is_open()) return false;
    ofs << id << "|" << name << "|" << (char)type << "|" << (char)genderGroup << "|" << maxParticipants << "|" << qualificationNum;
    // 保存积分规则
    for (int score : scoreRules) ofs << "|" << score;
    ofs << "|#"; // 分隔符
    // 保存参赛运动员
    for (int aid : participantIds) ofs << "|" << aid;
    ofs << std::endl;
    return ofs.good();
}

// 从文件加载
bool Event::loadFromFile(std::ifstream& ifs) {
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

    if (items.size() < 6) return false;

    int tempId, tempMax, tempQualification;
    char tempType, tempGender;
    if (!stringToInt(items[0], tempId)) return false;
    if (!stringToInt(items[4], tempMax)) return false;
    if (!stringToInt(items[5], tempQualification)) return false;
    tempType = items[2].empty() ? TRACK : items[2][0];
    tempGender = items[3].empty() ? MALE : items[3][0];

    id = tempId;
    name = items[1];
    type = (EventType)tempType;
    genderGroup = (Gender)tempGender;
    maxParticipants = tempMax;
    qualificationNum = tempQualification;
    scoreRules.clear();
    participantIds.clear();

    // 解析积分规则和参赛运动员
    bool isParticipantPart = false;
    for (size_t i = 6; i < items.size(); i++) {
        if (items[i] == "#") {
            isParticipantPart = true;
            continue;
        }
        if (!isParticipantPart) {
            int score;
            if (stringToInt(items[i], score)) {
                scoreRules.push_back(score);
            }
        }
        else {
            int aid;
            if (stringToInt(items[i], aid)) {
                participantIds.push_back(aid);
            }
        }
    }

    return true;
}

// 打印项目信息
void Event::printInfo() const {
    std::cout << std::left << std::setw(8) << id
        << std::setw(16) << name
        << std::setw(8) << (genderGroup == MALE ? "男子" : "女子")
        << std::setw(8) << (type == FIELD ? "田赛" : "径赛")
        << std::setw(12) << participantIds.size() << "/" << maxParticipants
        << std::setw(10) << (isEventCancelled() ? "已取消" : "正常")
        << std::endl;
}

// 打印参赛运动员列表
void Event::printParticipants() const {
    std::cout << "项目【" << name << "】参赛运动员列表（共" << participantIds.size() << "人）：" << std::endl;
    std::cout << std::left << std::setw(10) << "运动员编号" << std::setw(15) << "运动员姓名" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}