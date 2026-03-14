#include "Athlete.h"

// 构造函数
Athlete::Athlete() : id(0), gender(MALE), teamId(0), maxEvents(DEFAULT_MAX_EVENTS_PER_ATHLETE) {}

Athlete::Athlete(int id, const std::string& name, Gender gender, int teamId, int maxEvents)
    : id(id), name(name), gender(gender), teamId(teamId), maxEvents(maxEvents) {}

// Getter 实现
int Athlete::getId() const { return id; }
std::string Athlete::getName() const { return name; }
Gender Athlete::getGender() const { return gender; }
int Athlete::getTeamId() const { return teamId; }
const std::vector<int>& Athlete::getEventIds() const { return eventIds; }
int Athlete::getMaxEvents() const { return maxEvents; }
int Athlete::getRegisteredEventCount() const { return eventIds.size(); }

// Setter 实现
void Athlete::setId(int id) { this->id = id; }
void Athlete::setName(const std::string& name) { this->name = name; }
void Athlete::setGender(Gender gender) { this->gender = gender; }
void Athlete::setTeamId(int teamId) { this->teamId = teamId; }
void Athlete::setMaxEvents(int max) { this->maxEvents = max; }

// 报名项目添加
bool Athlete::addEvent(int eventId) {
    if (isRegistered(eventId)) return false;
    if (eventIds.size() >= maxEvents) return false;
    eventIds.push_back(eventId);
    return true;
}

// 取消报名项目
bool Athlete::removeEvent(int eventId) {
    auto it = std::find(eventIds.begin(), eventIds.end(), eventId);
    if (it == eventIds.end()) return false;
    eventIds.erase(it);
    return true;
}

// 检查是否已报名
bool Athlete::isRegistered(int eventId) const {
    return std::find(eventIds.begin(), eventIds.end(), eventId) != eventIds.end();
}

// 清空报名项目
void Athlete::clearEvents() {
    eventIds.clear();
}

// 保存到文件
bool Athlete::saveToFile(std::ofstream& ofs) const {
    if (!ofs.is_open()) return false;
    ofs << id << "|" << name << "|" << (char)gender << "|" << teamId << "|" << maxEvents;
    for (int eid : eventIds) ofs << "|" << eid;
    ofs << std::endl;
    return ofs.good();
}

// 从文件加载
bool Athlete::loadFromFile(std::ifstream& ifs) {
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

    int tempId, tempTeamId, tempMax;
    char tempGender;
    if (!stringToInt(items[0], tempId)) return false;
    if (!stringToInt(items[3], tempTeamId)) return false;
    if (!stringToInt(items[4], tempMax)) return false;
    tempGender = items[2].empty() ? MALE : items[2][0];

    id = tempId;
    name = items[1];
    gender = (Gender)tempGender;
    teamId = tempTeamId;
    maxEvents = tempMax;
    eventIds.clear();

    for (size_t i = 5; i < items.size(); i++) {
        int eid;
        if (stringToInt(items[i], eid)) {
            eventIds.push_back(eid);
        }
    }

    return true;
}

// 打印运动员信息
void Athlete::printInfo() const {
    std::cout << std::left << std::setw(8) << id
        << std::setw(12) << name
        << std::setw(8) << (gender == MALE ? "男" : "女")
        << std::setw(10) << teamId
        << std::setw(10) << eventIds.size() << "/" << maxEvents
        << std::endl;
}