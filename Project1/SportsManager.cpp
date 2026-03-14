#include "SportsManager.h"
#include <sys/stat.h>
//#include <dirent.h>

// 辅助函数：判断目录是否存在
bool SportsManager::isDirExist(const std::string& dirPath) const {
    return fs::exists(dirPath) && fs::is_directory(dirPath);
}

// 辅助函数：创建目录（支持递归创建多级目录）
bool SportsManager::createDir(const std::string& dirPath) const {
    if (isDirExist(dirPath)) {
        return true;  // 目录已存在，直接返回成功
    }
    try {
        return fs::create_directories(dirPath);  // 递归创建所有不存在的目录
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "创建目录失败: " << e.what() << std::endl;
        return false;
    }
}

// 获取下一个ID
int SportsManager::getNextTeamId() const {
    int maxId = 0;
    for (const auto& t : teams) {
        if (t.getId() > maxId) maxId = t.getId();
    }
    return maxId + 1;
}

int SportsManager::getNextEventId() const {
    int maxId = 0;
    for (const auto& e : events) {
        if (e.getId() > maxId) maxId = e.getId();
    }
    return maxId + 1;
}

int SportsManager::getNextAthleteId() const {
    int maxId = 0;
    for (const auto& a : athletes) {
        if (a.getId() > maxId) maxId = a.getId();
    }
    return maxId + 1;
}

// 构造函数
SportsManager::SportsManager(const std::string& dataDir) : dataDir(dataDir), maxEventsPerAthlete(DEFAULT_MAX_EVENTS_PER_ATHLETE) {}

// 系统初始化
bool SportsManager::initSystem() {
    // 创建数据目录
    if (!isDirExist(dataDir)) {
        if (!createDir(dataDir)) {
            std::cerr << "错误：无法创建数据目录" << std::endl;
            return false;
        }
    }
    // 加载所有数据
    return loadAllData();
}

// 重置系统
void SportsManager::resetSystem() {
    teams.clear();
    events.clear();
    athletes.clear();
    results.clear();
    schedule = Schedule();
    maxEventsPerAthlete = DEFAULT_MAX_EVENTS_PER_ATHLETE;
}

// 系统设置
void SportsManager::setMaxEventsPerAthlete(int max) {
    maxEventsPerAthlete = max > 0 ? max : DEFAULT_MAX_EVENTS_PER_ATHLETE;
    // 更新所有运动员的最大参赛数
    for (auto& a : athletes) {
        a.setMaxEvents(maxEventsPerAthlete);
    }
}

int SportsManager::getMaxEventsPerAthlete() const {
    return maxEventsPerAthlete;
}

void SportsManager::setScheduleConfig(int totalDays, int slotsPerMorning, int slotsPerAfternoon) {
    schedule.setTotalDays(totalDays);
    schedule.setSlotsPerMorning(slotsPerMorning);
    schedule.setSlotsPerAfternoon(slotsPerAfternoon);
}

void SportsManager::addScheduleVenue(const std::string& venue) {
    schedule.addVenue(venue);
}

// 参赛单位管理
bool SportsManager::addTeam(const std::string& name) {
    if (name.empty()) return false;
    // 检查名称是否重复
    for (const auto& t : teams) {
        if (t.getName() == name) return false;
    }
    Team newTeam(getNextTeamId(), name);
    teams.push_back(newTeam);
    saveAllData();
    return true;
}

bool SportsManager::removeTeam(int teamId) {
    auto it = std::find_if(teams.begin(), teams.end(), [teamId](const Team& t) {
        return t.getId() == teamId;
        });
    if (it == teams.end()) return false;

    // 删除该单位的所有运动员
    std::vector<int> athleteIds = it->getAthleteIds();
    for (int aid : athleteIds) {
        removeAthlete(aid);
    }

    teams.erase(it);
    saveAllData();
    return true;
}

bool SportsManager::modifyTeamName(int teamId, const std::string& newName) {
    if (newName.empty()) return false;
    Team* team = getTeamById(teamId);
    if (!team) return false;
    // 检查名称重复
    for (const auto& t : teams) {
        if (t.getId() != teamId && t.getName() == newName) return false;
    }
    team->setName(newName);
    saveAllData();
    return true;
}

const Team* SportsManager::getTeamById(int teamId) const {
    for (const auto& t : teams) {
        if (t.getId() == teamId) return &t;
    }
    return nullptr;
}

Team* SportsManager::getTeamById(int teamId) {
    for (auto& t : teams) {
        if (t.getId() == teamId) return &t;
    }
    return nullptr;
}

const std::vector<Team>& SportsManager::getAllTeams() const {
    return teams;
}

void SportsManager::printAllTeams() const {
    std::cout << "===================== 参赛单位列表 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "编号"
        << std::setw(20) << "单位名称"
        << std::setw(10) << "总积分"
        << std::setw(10) << "  男子积分 "
        << std::setw(10) << "  女子积分 "
        << std::setw(10) << "  运动员数 "
        << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    for (const auto& t : teams) {
        t.printInfo();
    }
    std::cout << "====================================================================================" << std::endl;
}

// 比赛项目管理
bool SportsManager::addEvent(const std::string& name, EventType type, Gender genderGroup, int maxParticipants) {
    if (name.empty()) return false;
    // 检查重复
    for (const auto& e : events) {
        if (e.getName() == name && e.getGenderGroup() == genderGroup) return false;
    }
    Event newEvent(getNextEventId(), name, type, genderGroup, maxParticipants);
    events.push_back(newEvent);
    saveAllData();
    return true;
}

bool SportsManager::removeEvent(int eventId) {
    auto it = std::find_if(events.begin(), events.end(), [eventId](const Event& e) {
        return e.getId() == eventId;
        });
    if (it == events.end()) return false;

    // 清除所有运动员对该项目的报名
    for (auto& a : athletes) {
        a.removeEvent(eventId);
    }
    // 删除该项目的所有成绩
    std::vector<Result> newResults;
    for (const auto& r : results) {
        if (r.getEventId() != eventId) {
            newResults.push_back(r);
        }
    }
    results.swap(newResults);

    events.erase(it);
    saveAllData();
    return true;
}

bool SportsManager::modifyEvent(int eventId, const std::string& name, EventType type, Gender genderGroup) {
    if (name.empty()) return false;
    Event* event = getEventById(eventId);
    if (!event) return false;
    // 检查重复
    for (const auto& e : events) {
        if (e.getId() != eventId && e.getName() == name && e.getGenderGroup() == genderGroup) return false;
    }
    event->setName(name);
    event->setType(type);
    event->setGenderGroup(genderGroup);
    saveAllData();
    return true;
}

const Event* SportsManager::getEventById(int eventId) const {
    for (const auto& e : events) {
        if (e.getId() == eventId) return &e;
    }
    return nullptr;
}

Event* SportsManager::getEventById(int eventId) {
    for (auto& e : events) {
        if (e.getId() == eventId) return &e;
    }
    return nullptr;
}

const std::vector<Event>& SportsManager::getAllEvents() const {
    return events;
}

void SportsManager::printAllEvents() const {
    std::cout << "===================== 比赛项目列表 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "编号"
        << std::setw(16) << "项目名称"
        << std::setw(8) << "性别"
        << std::setw(8) << "类型"
        << std::setw(12) << "参赛人数"
        << std::setw(10) << "状态"
        << std::endl;
    std::cout << "------------------------------------------------------------------------" << std::endl;
    for (const auto& e : events) {
        e.printInfo();
    }
    std::cout << "========================================================================" << std::endl;
}

void SportsManager::checkAndCancelInvalidEvents() {
    std::cout << "正在检查项目有效性..." << std::endl;
    int cancelCount = 0;
    for (auto& e : events) {
        if (e.isEventCancelled()) {
            cancelCount++;
            std::cout << "项目【" << e.getName() << "】参赛人数不足" << MIN_PARTICIPANTS_FOR_EVENT << "人，已取消" << std::endl;
        }
    }
    if (cancelCount == 0) {
        std::cout << "所有项目均符合参赛人数要求，无取消项目" << std::endl;
    }
    saveAllData();
}

// 运动员管理
bool SportsManager::addAthlete(const std::string& name, Gender gender, int teamId) {
    if (name.empty()) return false;
    Team* team = getTeamById(teamId);
    if (!team) return false;

    Athlete newAthlete(getNextAthleteId(), name, gender, teamId, maxEventsPerAthlete);
    athletes.push_back(newAthlete);
    team->addAthlete(newAthlete.getId());
    saveAllData();
    return true;
}

bool SportsManager::removeAthlete(int athleteId) {
    auto it = std::find_if(athletes.begin(), athletes.end(), [athleteId](const Athlete& a) {
        return a.getId() == athleteId;
        });
    if (it == athletes.end()) return false;

    int teamId = it->getTeamId();
    // 清除该运动员的所有报名
    std::vector<int> eventIds = it->getEventIds();
    for (int eid : eventIds) {
        Event* event = getEventById(eid);
        if (event) event->removeParticipant(athleteId);
    }
    // 删除该运动员的所有成绩
    std::vector<Result> newResults;
    for (const auto& r : results) {
        if (r.getAthleteId() != athleteId) {
            newResults.push_back(r);
        }
    }
    results.swap(newResults);
    // 从所属单位移除
    Team* team = getTeamById(teamId);
    if (team) team->removeAthlete(athleteId);

    athletes.erase(it);
    saveAllData();
    return true;
}

bool SportsManager::modifyAthlete(int athleteId, const std::string& name, Gender gender, int teamId) {
    if (name.empty()) return false;
    Athlete* athlete = getAthleteById(athleteId);
    if (!athlete) return false;
    Team* newTeam = getTeamById(teamId);
    if (!newTeam) return false;

    // 从原单位移除
    int oldTeamId = athlete->getTeamId();
    Team* oldTeam = getTeamById(oldTeamId);
    if (oldTeam) oldTeam->removeAthlete(athleteId);

    // 更新信息
    athlete->setName(name);
    athlete->setGender(gender);
    athlete->setTeamId(teamId);
    newTeam->addAthlete(athleteId);

    saveAllData();
    return true;
}

const Athlete* SportsManager::getAthleteById(int athleteId) const {
    for (const auto& a : athletes) {
        if (a.getId() == athleteId) return &a;
    }
    return nullptr;
}

Athlete* SportsManager::getAthleteById(int athleteId) {
    for (auto& a : athletes) {
        if (a.getId() == athleteId) return &a;
    }
    return nullptr;
}

const std::vector<Athlete>& SportsManager::getAllAthletes() const {
    return athletes;
}

void SportsManager::printAthletesByTeam(int teamId) const {
    const Team* team = getTeamById(teamId);
    if (!team) {
        std::cout << "参赛单位不存在" << std::endl;
        return;
    }
    std::cout << "===================== " << team->getName() << " 运动员列表 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "编号"
        << std::setw(12) << "姓名"
        << std::setw(8) << "性别"
        << std::setw(10) << "报名项目数"
        << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    for (int aid : team->getAthleteIds()) {
        const Athlete* a = getAthleteById(aid);
        if (a) a->printInfo();
    }
    std::cout << "========================================================" << std::endl;
}

void SportsManager::printAllAthletes() const {
    std::cout << "===================== 所有运动员列表 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "编号"
        << std::setw(12) << "姓名"
        << std::setw(8) << "性别"
        << std::setw(10) << "所属单位ID"<<" "
        << std::setw(10) << "报名项目数"
        << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    for (const auto& a : athletes) {
        a.printInfo();
    }
    std::cout << "====================================================================" << std::endl;
}

// 报名管理
bool SportsManager::athleteRegisterEvent(int athleteId, int eventId) {
    Athlete* athlete = getAthleteById(athleteId);
    Event* event = getEventById(eventId);
    if (!athlete || !event) return false;

    // 性别校验
    if (athlete->getGender() != event->getGenderGroup()) {
        std::cerr << "错误：运动员性别与项目性别分组不符" << std::endl;
        return false;
    }

    // 报名校验
    if (!athlete->addEvent(eventId)) {
        std::cerr << "错误：运动员报名失败（已达报名上限或已报名该项目）" << std::endl;
        return false;
    }
    if (!event->addParticipant(athleteId)) {
        athlete->removeEvent(eventId);
        std::cerr << "错误：项目报名人数已满" << std::endl;
        return false;
    }

    saveAllData();
    std::cout << "报名成功！" << std::endl;
    return true;
}

bool SportsManager::athleteCancelRegister(int athleteId, int eventId) {
    Athlete* athlete = getAthleteById(athleteId);
    Event* event = getEventById(eventId);
    if (!athlete || !event) return false;

    if (!athlete->removeEvent(eventId) || !event->removeParticipant(athleteId)) {
        std::cerr << "错误：取消报名失败" << std::endl;
        return false;
    }

    // 删除该运动员在该项目的成绩
    std::vector<Result> newResults;
    for (const auto& r : results) {
        if (!(r.getEventId() == eventId && r.getAthleteId() == athleteId)) {
            newResults.push_back(r);
        }
    }
    results.swap(newResults);

    saveAllData();
    std::cout << "取消报名成功！" << std::endl;
    return true;
}

void SportsManager::printAthleteRegisterInfo(int athleteId) const {
    const Athlete* athlete = getAthleteById(athleteId);
    if (!athlete) {
        std::cout << "运动员不存在" << std::endl;
        return;
    }
    std::cout << "===================== 运动员【" << athlete->getName() << "】报名信息 =====================" << std::endl;
    std::cout << "已报名项目数：" << athlete->getRegisteredEventCount() << "/" << athlete->getMaxEvents() << std::endl;
    std::cout << std::left << std::setw(8) << "项目编号" << std::setw(20) << "项目名称" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (int eid : athlete->getEventIds()) {
        const Event* event = getEventById(eid);
        if (event) {
            std::cout << std::left << std::setw(8) << eid << std::setw(20) << event->getName() << std::endl;
        }
    }
    std::cout << "========================================================" << std::endl;
}

void SportsManager::printEventRegisterInfo(int eventId) const {
    const Event* event = getEventById(eventId);
    if (!event) {
        std::cout << "项目不存在!" << std::endl;
        return;
    }
    std::cout << "===================== 项目【" << event->getName() << "】报名信息 =====================" << std::endl;
    std::cout << "参赛人数：" << event->getParticipantCount() << "/" << event->getMaxParticipants() << std::endl;
    std::cout << "项目状态：" << (event->isEventCancelled() ? "已取消" : "正常") << std::endl;
    std::cout << std::left << std::setw(8) << "运动员编号" << std::setw(12) << "姓名" << std::setw(20) << "所属单位" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    for (int aid : event->getParticipantIds()) {
        const Athlete* a = getAthleteById(aid);
        const Team* t = getTeamById(a->getTeamId());
        if (a && t) {
            std::cout << std::left << std::setw(8) << aid
                << std::setw(12) << a->getName()
                << std::setw(20) << t->getName()
                << std::endl;
        }
    }
    std::cout << "========================================================" << std::endl;
}

// 秩序册管理
bool SportsManager::generateSchedule() {
    bool res = schedule.generateSchedule(events, athletes);
    if (res) {
        saveAllData();
    }
    return res;
}

void SportsManager::printFullSchedule() const {
    schedule.printFullSchedule();
}

void SportsManager::exportSchedule(const std::string& filePath) const {
    schedule.exportScheduleToTxt(filePath);
}

// 成绩管理
bool SportsManager::addResult(int eventId, int athleteId, const std::string& result) {
    const Event* event = getEventById(eventId);
    const Athlete* athlete = getAthleteById(athleteId);
    if (!event || !athlete) return false;
    if (!event->isParticipant(athleteId)) {
        std::cerr << "错误：该运动员未报名此项目" << std::endl;
        return false;
    }
    // 检查是否已存在成绩
    for (const auto& r : results) {
        if (r.getEventId() == eventId && r.getAthleteId() == athleteId) {
            std::cerr << "错误：该运动员已存在此项目的成绩" << std::endl;
            return false;
        }
    }

    Result newResult(eventId, athleteId, result);
    results.push_back(newResult);
    // 重新计算该项目的名次和得分
    calculateEventResult(eventId);
    saveAllData();
    std::cout << "成绩录入成功！" << std::endl;
    return true;
}

bool SportsManager::modifyResult(int eventId, int athleteId, const std::string& newResult) {
    for (auto& r : results) {
        if (r.getEventId() == eventId && r.getAthleteId() == athleteId) {
            r.setResult(newResult);
            calculateEventResult(eventId);
            saveAllData();
            std::cout << "成绩修改成功！" << std::endl;
            return true;
        }
    }
    std::cerr << "错误：未找到对应成绩记录" << std::endl;
    return false;
}

bool SportsManager::removeResult(int eventId, int athleteId) {
    auto it = std::find_if(results.begin(), results.end(), [eventId, athleteId](const Result& r) {
        return r.getEventId() == eventId && r.getAthleteId() == athleteId;
        });
    if (it == results.end()) {
        std::cerr << "错误：未找到对应成绩记录" << std::endl;
        return false;
    }
    results.erase(it);
    calculateEventResult(eventId);
    saveAllData();
    std::cout << "成绩删除成功！" << std::endl;
    return true;
}

// 计算单个项目的名次和得分
void SportsManager::calculateEventResult(int eventId) {
    Event* event = getEventById(eventId);
    if (!event) return;

    // 获取该项目的所有成绩
    std::vector<Result*> eventResults;
    for (auto& r : results) {
        if (r.getEventId() == eventId) {
            eventResults.push_back(&r);
        }
    }

    // 按成绩排序
    std::sort(eventResults.begin(), eventResults.end(), [event](Result* a, Result* b) {
        return event->isBetterResult(a->getResult(), b->getResult());
        });

    // 设置名次和得分
    event->autoSetScoreRules();
    int qualificationNum = event->getQualificationNum();
    const std::vector<int>& scoreRules = event->getScoreRules();

    for (size_t i = 0; i < eventResults.size(); i++) {
        int rank = i + 1;
        eventResults[i]->setRank(rank);
        if (rank <= qualificationNum) {
            eventResults[i]->setScore(scoreRules[rank - 1]);
        }
        else {
            eventResults[i]->setScore(0);
        }
    }

    // 更新单位总分
    updateTeamTotalScore();
}

// 计算所有项目的名次和得分
void SportsManager::calculateAllRankAndScore() {
    for (const auto& e : events) {
        calculateEventResult(e.getId());
    }
    std::cout << "所有项目名次和得分计算完成！" << std::endl;
}

// 更新单位总分
void SportsManager::updateTeamTotalScore() {
    // 重置所有单位分数
    for (auto& t : teams) {
        t.resetScore();
    }

    // 累计得分
    for (const auto& r : results) {
        if (r.getScore() <= 0) continue;
        const Athlete* a = getAthleteById(r.getAthleteId());
        Team* t = getTeamById(a->getTeamId());
        if (a && t) {
            t->addScore(r.getScore(), a->getGender());
        }
    }
}

const Result* SportsManager::getResult(int eventId, int athleteId) const {
    for (const auto& r : results) {
        if (r.getEventId() == eventId && r.getAthleteId() == athleteId) {
            return &r;
        }
    }
    return nullptr;
}

std::vector<Result> SportsManager::getResultsByEvent(int eventId) const {
    std::vector<Result> res;
    for (const auto& r : results) {
        if (r.getEventId() == eventId) {
            res.push_back(r);
        }
    }
    // 按名次排序
    std::sort(res.begin(), res.end(), [](const Result& a, const Result& b) {
        return a.getRank() < b.getRank();
        });
    return res;
}

std::vector<Result> SportsManager::getResultsByAthlete(int athleteId) const {
    std::vector<Result> res;
    for (const auto& r : results) {
        if (r.getAthleteId() == athleteId) {
            res.push_back(r);
        }
    }
    return res;
}

std::vector<Result> SportsManager::getResultsByTeam(int teamId) const {
    std::vector<Result> res;
    const Team* team = getTeamById(teamId);
    if (!team) return res;

    for (int aid : team->getAthleteIds()) {
        auto athleteResults = getResultsByAthlete(aid);
        res.insert(res.end(), athleteResults.begin(), athleteResults.end());
    }
    return res;
}

// 成绩查询打印
void SportsManager::printEventResult(int eventId) const {
    const Event* event = getEventById(eventId);
    if (!event) {
        std::cout << "项目不存在" << std::endl;
        return;
    }
    auto eventResults = getResultsByEvent(eventId);
    std::cout << "===================== 【" << event->getName() << "】比赛成绩 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "名次"
        << std::setw(12) << "姓名"
        << std::setw(20) << "所属单位"
        << std::setw(12) << "成绩"
        << std::setw(8) << "得分"
        << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    for (const auto& r : eventResults) {
        const Athlete* a = getAthleteById(r.getAthleteId());
        const Team* t = getTeamById(a->getTeamId());
        if (a && t) {
            std::cout << std::left << std::setw(8) << r.getRank()
                << std::setw(12) << a->getName()
                << std::setw(20) << t->getName()
                << std::setw(12) << r.getResult()
                << std::setw(8) << r.getScore()
                << std::endl;
        }
    }
    std::cout << "====================================================================================" << std::endl;
}

void SportsManager::printAthleteResult(int athleteId) const {
    const Athlete* athlete = getAthleteById(athleteId);
    if (!athlete) {
        std::cout << "运动员不存在" << std::endl;
        return;
    }
    auto athleteResults = getResultsByAthlete(athleteId);
    std::cout << "===================== 运动员【" << athlete->getName() << "】比赛成绩 =====================" << std::endl;
    std::cout << std::left << std::setw(20) << "项目名称"
        << std::setw(8) << "名次"
        << std::setw(12) << "成绩"
        << std::setw(8) << "得分"
        << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    int totalScore = 0;
    for (const auto& r : athleteResults) {
        const Event* e = getEventById(r.getEventId());
        if (e) {
            totalScore += r.getScore();
            std::cout << std::left << std::setw(20) << e->getName()
                << std::setw(8) << r.getRank()
                << std::setw(12) << r.getResult()
                << std::setw(8) << r.getScore()
                << std::endl;
        }
    }
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "总得分：" << totalScore << std::endl;
    std::cout << "========================================================" << std::endl;
}

void SportsManager::printTeamResult(int teamId) const {
    const Team* team = getTeamById(teamId);
    if (!team) {
        std::cout << "参赛单位不存在" << std::endl;
        return;
    }
    std::cout << "===================== 【" << team->getName() << "】比赛成绩汇总 =====================" << std::endl;
    std::cout << "团体总积分：" << team->getTotalScore() << std::endl;
    std::cout << "男子团体积分：" << team->getMenScore() << std::endl;
    std::cout << "女子团体积分：" << team->getWomenScore() << std::endl;
    std::cout << std::endl;
    std::cout << "运动员成绩明细：" << std::endl;
    std::cout << std::left << std::setw(12) << "运动员姓名"
        << std::setw(20) << "项目名称"
        << std::setw(8) << "名次"
        << std::setw(12) << "成绩"
        << std::setw(8) << "得分"
        << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    for (int aid : team->getAthleteIds()) {
        const Athlete* a = getAthleteById(aid);
        auto results = getResultsByAthlete(aid);
        for (const auto& r : results) {
            const Event* e = getEventById(r.getEventId());
            if (a && e) {
                std::cout << std::left << std::setw(12) << a->getName()
                    << std::setw(20) << e->getName()
                    << std::setw(8) << r.getRank()
                    << std::setw(12) << r.getResult()
                    << std::setw(8) << r.getScore()
                    << std::endl;
            }
        }
    }
    std::cout << "====================================================================================" << std::endl;
}

void SportsManager::printTeamRank() const {
    std::vector<Team> sortedTeams = teams;
    std::sort(sortedTeams.begin(), sortedTeams.end(), [](const Team& a, const Team& b) {
        return a.getTotalScore() > b.getTotalScore();
        });

    std::cout << "===================== 团体总分排行榜 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "排名"
        << std::setw(20) << "单位名称"
        << std::setw(10) << "总积分"
        << std::setw(10) << "男子积分"
        << std::setw(10) << "女子积分"
        << std::endl;
    std::cout << "------------------------------------------------------------------------------------" << std::endl;
    for (size_t i = 0; i < sortedTeams.size(); i++) {
        std::cout << std::left << std::setw(8) << (i + 1);
        sortedTeams[i].printScoreRank();
    }
    std::cout << "====================================================================================" << std::endl;
}

void SportsManager::printGenderGroupRank(Gender gender) const {
    std::vector<Team> sortedTeams = teams;
    std::sort(sortedTeams.begin(), sortedTeams.end(), [gender](const Team& a, const Team& b) {
        return gender == MALE ? a.getMenScore() > b.getMenScore() : a.getWomenScore() > b.getWomenScore();
        });

    std::cout << "===================== " << (gender == MALE ? "男子" : "女子") << "团体排行榜 =====================" << std::endl;
    std::cout << std::left << std::setw(8) << "排名"
        << std::setw(20) << "单位名称"
        << std::setw(10) << (gender == MALE ? "男子积分" : "女子积分")
        << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    for (size_t i = 0; i < sortedTeams.size(); i++) {
        std::cout << std::left << std::setw(8) << (i + 1)
            << std::setw(20) << sortedTeams[i].getName()
            << std::setw(10) << (gender == MALE ? sortedTeams[i].getMenScore() : sortedTeams[i].getWomenScore())
            << std::endl;
    }
    std::cout << "========================================================" << std::endl;
}

// 数据加载与保存
bool SportsManager::loadAllData() {
    // 加载参赛单位
    std::ifstream teamFile(dataDir + "/teams.txt");
    if (teamFile.is_open()) {
        Team team;
        while (team.loadFromFile(teamFile)) {
            teams.push_back(team);
        }
        teamFile.close();
    }

    // 加载比赛项目
    std::ifstream eventFile(dataDir + "/events.txt");
    if (eventFile.is_open()) {
        Event event;
        while (event.loadFromFile(eventFile)) {
            events.push_back(event);
        }
        eventFile.close();
    }

    // 加载运动员
    std::ifstream athleteFile(dataDir + "/athletes.txt");
    if (athleteFile.is_open()) {
        Athlete athlete;
        while (athlete.loadFromFile(athleteFile)) {
            athletes.push_back(athlete);
        }
        athleteFile.close();
    }

    // 加载成绩
    std::ifstream resultFile(dataDir + "/results.txt");
    if (resultFile.is_open()) {
        Result result;
        while (result.loadFromFile(resultFile)) {
            results.push_back(result);
        }
        resultFile.close();
    }

    // 加载秩序册
    std::ifstream scheduleFile(dataDir + "/schedule.txt");
    if (scheduleFile.is_open()) {
        schedule.loadFromFile(scheduleFile);
        scheduleFile.close();
    }

    // 加载系统配置
    std::ifstream configFile(dataDir + "/config.txt");
    if (configFile.is_open()) {
        std::string line;
        if (std::getline(configFile, line)) {
            int maxEvents;
            if (stringToInt(line, maxEvents)) {
                maxEventsPerAthlete = maxEvents;
            }
        }
        configFile.close();
    }

    std::cout << "系统数据加载完成！" << std::endl;
    return true;
}

bool SportsManager::saveAllData() const {
    // 保存参赛单位
    std::ofstream teamFile(dataDir + "/teams.txt");
    if (teamFile.is_open()) {
        for (const auto& t : teams) {
            t.saveToFile(teamFile);
        }
        teamFile.close();
    }

    // 保存比赛项目
    std::ofstream eventFile(dataDir + "/events.txt");
    if (eventFile.is_open()) {
        for (const auto& e : events) {
            e.saveToFile(eventFile);
        }
        eventFile.close();
    }

    // 保存运动员
    std::ofstream athleteFile(dataDir + "/athletes.txt");
    if (athleteFile.is_open()) {
        for (const auto& a : athletes) {
            a.saveToFile(athleteFile);
        }
        athleteFile.close();
    }

    // 保存成绩
    std::ofstream resultFile(dataDir + "/results.txt");
    if (resultFile.is_open()) {
        for (const auto& r : results) {
            r.saveToFile(resultFile);
        }
        resultFile.close();
    }

    // 保存秩序册
    std::ofstream scheduleFile(dataDir + "/schedule.txt");
    if (scheduleFile.is_open()) {
        schedule.saveToFile(scheduleFile);
        scheduleFile.close();
    }

    // 保存系统配置
    std::ofstream configFile(dataDir + "/config.txt");
    if (configFile.is_open()) {
        configFile << maxEventsPerAthlete << std::endl;
        configFile.close();
    }

    return true;
}

// 数据备份与恢复
bool SportsManager::backupData(const std::string& backupDir) const {
    if (!isDirExist(backupDir)) {
        if (!createDir(backupDir)) {
            std::cerr << "错误：无法创建备份目录" << std::endl;
            return false;
        }
    }

    // 复制所有数据文件
    std::vector<std::string> files = { "teams.txt", "events.txt", "athletes.txt", "results.txt", "schedule.txt", "config.txt" };
    for (const auto& file : files) {
        std::ifstream src(dataDir + "/" + file, std::ios::binary);
        std::ofstream dst(backupDir + "/" + file, std::ios::binary);
        if (src.is_open() && dst.is_open()) {
            dst << src.rdbuf();
            src.close();
            dst.close();
        }
    }

    std::cout << "数据备份成功！备份目录：" << backupDir << std::endl;
    return true;
}

bool SportsManager::restoreData(const std::string& backupDir) {
    if (!isDirExist(backupDir)) {
        std::cerr << "错误：备份目录不存在" << std::endl;
        return false;
    }

    resetSystem();

    // 复制所有备份文件到数据目录
    std::vector<std::string> files = { "teams.txt", "events.txt", "athletes.txt", "results.txt", "schedule.txt", "config.txt" };
    for (const auto& file : files) {
        std::ifstream src(backupDir + "/" + file, std::ios::binary);
        std::ofstream dst(dataDir + "/" + file, std::ios::binary);
        if (src.is_open() && dst.is_open()) {
            dst << src.rdbuf();
            src.close();
            dst.close();
        }
    }

    // 重新加载数据
    loadAllData();
    std::cout << "数据恢复成功！" << std::endl;
    return true;
}

bool SportsManager::exportDataToTxt(const std::string& exportDir) const {
    if (!isDirExist(exportDir)) {
        if (!createDir(exportDir)) {
            std::cerr << "错误：无法创建导出目录" << std::endl;
            return false;
        }
    }

    // 导出参赛单位
    std::ofstream teamExport(exportDir + "/参赛单位列表.txt");
    if (teamExport.is_open()) {
        teamExport << "===================== 参赛单位列表 =====================" << std::endl;
        teamExport << std::left << std::setw(8) << "编号"
            << std::setw(20) << "单位名称"
            << std::setw(10) << "总积分"
            << std::setw(10) << "男子积分"
            << std::setw(10) << "女子积分"
            << std::setw(10) << "运动员数"
            << std::endl;
        teamExport << "------------------------------------------------------------------------------------" << std::endl;
        for (const auto& t : teams) {
            teamExport << std::left << std::setw(8) << t.getId()
                << std::setw(20) << t.getName()
                << std::setw(10) << t.getTotalScore()
                << std::setw(10) << t.getMenScore()
                << std::setw(10) << t.getWomenScore()
                << std::setw(10) << t.getAthleteIds().size()
                << std::endl;
        }
        teamExport.close();
    }

    // 导出项目列表
    std::ofstream eventExport(exportDir + "/比赛项目列表.txt");
    if (eventExport.is_open()) {
        eventExport << "===================== 比赛项目列表 =====================" << std::endl;
        eventExport << std::left << std::setw(8) << "编号"
            << std::setw(16) << "项目名称"
            << std::setw(8) << "性别"
            << std::setw(8) << "类型"
            << std::setw(12) << "参赛人数"
            << std::setw(10) << "状态"
            << std::endl;
        eventExport << "------------------------------------------------------------------------" << std::endl;
        for (const auto& e : events) {
            eventExport << std::left << std::setw(8) << e.getId()
                << std::setw(16) << e.getName()
                << std::setw(8) << (e.getGenderGroup() == MALE ? "男子" : "女子")
                << std::setw(8) << (e.getType() == FIELD ? "田赛" : "径赛")
                << std::setw(12) << std::to_string(e.getParticipantCount()) + "/" + std::to_string(e.getMaxParticipants())
                << std::setw(10) << (e.isEventCancelled() ? "已取消" : "正常")
                << std::endl;
        }
        eventExport.close();
    }

    // 导出排行榜
    std::ofstream rankExport(exportDir + "/团体排行榜.txt");
    if (rankExport.is_open()) {
        std::vector<Team> sortedTeams = teams;
        std::sort(sortedTeams.begin(), sortedTeams.end(), [](const Team& a, const Team& b) {
            return a.getTotalScore() > b.getTotalScore();
            });
        rankExport << "===================== 团体总分排行榜 =====================" << std::endl;
        rankExport << std::left << std::setw(8) << "排名"
            << std::setw(20) << "单位名称"
            << std::setw(10) << "总积分"
            << std::setw(10) << "男子积分"
            << std::setw(10) << "女子积分"
            << std::endl;
        rankExport << "------------------------------------------------------------------------------------" << std::endl;
        for (size_t i = 0; i < sortedTeams.size(); i++) {
            rankExport << std::left << std::setw(8) << (i + 1)
                << std::setw(20) << sortedTeams[i].getName()
                << std::setw(10) << sortedTeams[i].getTotalScore()
                << std::setw(10) << sortedTeams[i].getMenScore()
                << std::setw(10) << sortedTeams[i].getWomenScore()
                << std::endl;
        }
        rankExport.close();
    }

    // 导出秩序册
    schedule.exportScheduleToTxt(exportDir + "/运动会秩序册.txt");

    std::cout << "数据导出成功！导出目录：" << exportDir << std::endl;
    return true;
}

// 系统主菜单
void SportsManager::showMainMenu() {
    std::cout << std::endl;
    std::cout << "===================== 学校运动会管理系统 =====================" << std::endl;
    std::cout << "                                                            " << std::endl;
    std::cout << "1.  系统设置" << std::endl;
    std::cout << "2.  参赛单位管理（哪个学院？）" << std::endl;
    std::cout << "3.  比赛项目管理" << std::endl;
    std::cout << "4.  运动员管理" << std::endl;
    std::cout << "5.  运动员报名登记" << std::endl;
    std::cout << "6.  参赛信息查询" << std::endl;
    std::cout << "7.  秩序册自动生成" << std::endl;
    std::cout << "8.  比赛成绩录入与管理" << std::endl;
    std::cout << "9.  比赛成绩查询" << std::endl;
    std::cout << "10. 成绩统计与排名" << std::endl;
    std::cout << "11. 数据备份与恢复" << std::endl;
    std::cout << "0.  退出系统" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "请选择操作：";
}

// 系统运行主循环
void SportsManager::run() {
    std::cout << "欢迎使用学校运动会管理系统！" << std::endl;
    if (!initSystem()) {
        std::cerr << "系统初始化失败！" << std::endl;
        return;
    }

    int choice;
    while (true) {
        showMainMenu();
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            std::cout << "输入无效，请输入数字！" << std::endl;
            continue;
        }

        switch (choice) {
        case 0:
            saveAllData();
            std::cout << "系统已保存，感谢使用！" << std::endl;
            return;
        case 1: {
            // 系统设置
            std::cout << "===================== 系统设置 =====================" << std::endl;
            std::cout << "1. 设置运动员最多参赛项目数" << std::endl;
            std::cout << "2. 设置赛程参数" << std::endl;
            std::cout << "3. 添加比赛场地" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                int max;
                std::cout << "请输入运动员最多参赛项目数：";
                std::cin >> max;
                setMaxEventsPerAthlete(max);
                std::cout << "设置成功！当前最多参赛项目数：" << maxEventsPerAthlete << std::endl;
            }
            else if (subChoice == 2) {
                int days, morning, afternoon;
                std::cout << "请输入比赛总天数：";
                std::cin >> days;
                std::cout << "请输入上午最多可安排项目数：";
                std::cin >> morning;
                std::cout << "请输入下午最多可安排项目数：";
                std::cin >> afternoon;
                setScheduleConfig(days, morning, afternoon);
                std::cout << "赛程参数设置成功！" << std::endl;
            }
            else if (subChoice == 3) {
                std::string venue;
                std::cout << "请输入比赛场地名称：";
                std::cin >> venue;
                addScheduleVenue(venue);
                std::cout << "场地添加成功！" << std::endl;
            }
            break;
        }
        case 2: {
            // 参赛单位管理
            std::cout << "===================== 参赛单位管理 =====================" << std::endl;
            std::cout << "1. 添加参赛单位" << std::endl;
            std::cout << "2. 删除参赛单位" << std::endl;
            std::cout << "3. 修改参赛单位名称" << std::endl;
            std::cout << "4. 查看所有参赛单位" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                std::string name;
                std::cout << "请输入参赛单位名称：";
                std::cin >> name;
                if (addTeam(name)) {
                    std::cout << "添加成功！" << std::endl;
                }
                else {
                    std::cout << "添加失败！" << std::endl;
                }
            }
            else if (subChoice == 2) {
                int id;
                std::cout << "请输入要删除的参赛单位编号：";
                std::cin >> id;
                if (removeTeam(id)) {
                    std::cout << "删除成功！" << std::endl;
                }
                else {
                    std::cout << "删除失败！" << std::endl;
                }
            }
            else if (subChoice == 3) {
                int id;
                std::string newName;
                std::cout << "请输入要修改的参赛单位编号：";
                std::cin >> id;
                std::cout << "请输入新的单位名称：";
                std::cin >> newName;
                if (modifyTeamName(id, newName)) {
                    std::cout << "修改成功！" << std::endl;
                }
                else {
                    std::cout << "修改失败！" << std::endl;
                }
            }
            else if (subChoice == 4) {
                printAllTeams();
            }
            break;
        }
        case 3: {
            // 比赛项目管理
            std::cout << "===================== 比赛项目管理 =====================" << std::endl;
            std::cout << "1. 添加比赛项目" << std::endl;
            std::cout << "2. 删除比赛项目" << std::endl;
            std::cout << "3. 修改比赛项目" << std::endl;
            std::cout << "4. 查看所有比赛项目" << std::endl;
            std::cout << "5. 检查并取消无效项目" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                std::string name;
                char type, gender;
                int max;
                std::cout << "请输入项目名称：";
                std::cin >> name;
                std::cout << "请输入项目类型（T-田赛 R-径赛）：";
                std::cin >> type;
                std::cout << "请输入性别分组（M-男子 F-女子）：";
                std::cin >> gender;
                std::cout << "请输入最大参赛人数：";
                std::cin >> max;
                if (addEvent(name, (EventType)type, (Gender)gender, max)) {
                    std::cout << "添加成功！" << std::endl;
                }
                else {
                    std::cout << "添加失败！" << std::endl;
                }
            }
            else if (subChoice == 2) {
                int id;
                std::cout << "请输入要删除的项目编号：";
                std::cin >> id;
                if (removeEvent(id)) {
                    std::cout << "删除成功！" << std::endl;
                }
                else {
                    std::cout << "删除失败！" << std::endl;
                }
            }
            else if (subChoice == 3) {
                int id;
                std::string name;
                char type, gender;
                std::cout << "请输入要修改的项目编号：";
                std::cin >> id;
                std::cout << "请输入新的项目名称：";
                std::cin >> name;
                std::cout << "请输入新的项目类型（T-田赛 R-径赛）：";
                std::cin >> type;
                std::cout << "请输入新的性别分组（M-男子 F-女子）：";
                std::cin >> gender;
                if (modifyEvent(id, name, (EventType)type, (Gender)gender)) {
                    std::cout << "修改成功！" << std::endl;
                }
                else {
                    std::cout << "修改失败！" << std::endl;
                }
            }
            else if (subChoice == 4) {
                printAllEvents();
            }
            else if (subChoice == 5) {
                checkAndCancelInvalidEvents();
            }
            break;
        }
        case 4: {
            // 运动员管理
            std::cout << "===================== 运动员管理 =====================" << std::endl;
            std::cout << "1. 添加运动员" << std::endl;
            std::cout << "2. 删除运动员" << std::endl;
            std::cout << "3. 修改运动员信息" << std::endl;
            std::cout << "4. 查看所有运动员" << std::endl;
            std::cout << "5. 按单位查看运动员" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                std::string name;
                char gender;
                int teamId;
                std::cout << "请输入运动员姓名：";
                std::cin >> name;
                std::cout << "请输入性别（M-男 F-女）：";
                std::cin >> gender;
                std::cout << "请输入所属单位编号：";
                std::cin >> teamId;
                if (addAthlete(name, (Gender)gender, teamId)) {
                    std::cout << "添加成功！" << std::endl;
                }
                else {
                    std::cout << "添加失败！" << std::endl;
                }
            }
            else if (subChoice == 2) {
                int id;
                std::cout << "请输入要删除的运动员编号：";
                std::cin >> id;
                if (removeAthlete(id)) {
                    std::cout << "删除成功！" << std::endl;
                }
                else {
                    std::cout << "删除失败！" << std::endl;
                }
            }
            else if (subChoice == 3) {
                int id;
                std::string name;
                char gender;
                int teamId;
                std::cout << "请输入要修改的运动员编号：";
                std::cin >> id;
                std::cout << "请输入新的姓名：";
                std::cin >> name;
                std::cout << "请输入新的性别（M-男 F-女）：";
                std::cin >> gender;
                std::cout << "请输入新的所属单位编号：";
                std::cin >> teamId;
                if (modifyAthlete(id, name, (Gender)gender, teamId)) {
                    std::cout << "修改成功！" << std::endl;
                }
                else {
                    std::cout << "修改失败！" << std::endl;
                }
            }
            else if (subChoice == 4) {
                printAllAthletes();
            }
            else if (subChoice == 5) {
                int teamId;
                std::cout << "请输入参赛单位编号：";
                std::cin >> teamId;
                printAthletesByTeam(teamId);
            }
            break;
        }
        case 5: {
            // 运动员报名登记
            std::cout << "===================== 运动员报名登记 =====================" << std::endl;
            std::cout << "1. 运动员报名项目" << std::endl;
            std::cout << "2. 运动员取消报名" << std::endl;
            std::cout << "3. 查看运动员报名信息" << std::endl;
            std::cout << "4. 查看项目报名信息" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                int athleteId, eventId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                athleteRegisterEvent(athleteId, eventId);
            }
            else if (subChoice == 2) {
                int athleteId, eventId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                athleteCancelRegister(athleteId, eventId);
            }
            else if (subChoice == 3) {
                int athleteId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                printAthleteRegisterInfo(athleteId);
            }
            else if (subChoice == 4) {
                int eventId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                printEventRegisterInfo(eventId);
            }
            break;
        }
        case 6: {
            // 参赛信息查询
            std::cout << "===================== 参赛信息查询 =====================" << std::endl;
            std::cout << "1. 查询所有参赛单位" << std::endl;
            std::cout << "2. 查询所有比赛项目" << std::endl;
            std::cout << "3. 查询所有运动员" << std::endl;
            std::cout << "4. 查询运动员报名信息" << std::endl;
            std::cout << "5. 查询项目报名信息" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                printAllTeams();
            }
            else if (subChoice == 2) {
                printAllEvents();
            }
            else if (subChoice == 3) {
                printAllAthletes();
            }
            else if (subChoice == 4) {
                int athleteId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                printAthleteRegisterInfo(athleteId);
            }
            else if (subChoice == 5) {
                int eventId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                printEventRegisterInfo(eventId);
            }
            break;
        }
        case 7: {
            // 秩序册自动生成
            std::cout << "===================== 秩序册自动生成 =====================" << std::endl;
            std::cout << "1. 生成秩序册" << std::endl;
            std::cout << "2. 查看完整秩序册" << std::endl;
            std::cout << "3. 按天查看赛程" << std::endl;
            std::cout << "4. 导出秩序册" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                generateSchedule();
            }
            else if (subChoice == 2) {
                printFullSchedule();
            }
            else if (subChoice == 3) {
                int day;
                std::cout << "请输入要查看的天数：";
                std::cin >> day;
                schedule.printScheduleByDay(day);
            }
            else if (subChoice == 4) {
                std::string path;
                std::cout << "请输入导出文件路径：";
                std::cin >> path;
                exportSchedule(path);
            }
            break;
        }
        case 8: {
            // 比赛成绩录入与管理
            std::cout << "===================== 比赛成绩录入与管理 =====================" << std::endl;
            std::cout << "1. 录入成绩" << std::endl;
            std::cout << "2. 修改成绩" << std::endl;
            std::cout << "3. 删除成绩" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                int eventId, athleteId;
                std::string result;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                std::cout << "请输入比赛成绩（数值）：";
                std::cin >> result;
                addResult(eventId, athleteId, result);
            }
            else if (subChoice == 2) {
                int eventId, athleteId;
                std::string newResult;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                std::cout << "请输入新的比赛成绩：";
                std::cin >> newResult;
                modifyResult(eventId, athleteId, newResult);
            }
            else if (subChoice == 3) {
                int eventId, athleteId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                removeResult(eventId, athleteId);
            }
            break;
        }
        case 9: {
            // 比赛成绩查询
            std::cout << "===================== 比赛成绩查询 =====================" << std::endl;
            std::cout << "1. 按项目查询成绩" << std::endl;
            std::cout << "2. 按运动员查询成绩" << std::endl;
            std::cout << "3. 按参赛单位查询成绩" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                int eventId;
                std::cout << "请输入项目编号：";
                std::cin >> eventId;
                printEventResult(eventId);
            }
            else if (subChoice == 2) {
                int athleteId;
                std::cout << "请输入运动员编号：";
                std::cin >> athleteId;
                printAthleteResult(athleteId);
            }
            else if (subChoice == 3) {
                int teamId;
                std::cout << "请输入参赛单位编号：";
                std::cin >> teamId;
                printTeamResult(teamId);
            }
            break;
        }
        case 10: {
            // 成绩统计与排名
            std::cout << "===================== 成绩统计与排名 =====================" << std::endl;
            std::cout << "1. 查看团体总分排行榜" << std::endl;
            std::cout << "2. 查看男子团体排行榜" << std::endl;
            std::cout << "3. 查看女子团体排行榜" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                printTeamRank();
            }
            else if (subChoice == 2) {
                printGenderGroupRank(MALE);
            }
            else if (subChoice == 3) {
                printGenderGroupRank(FEMALE);
            }
            break;
        }
        case 11: {
            // 数据备份与恢复
            std::cout << "===================== 数据备份与恢复 =====================" << std::endl;
            std::cout << "1. 数据备份" << std::endl;
            std::cout << "2. 数据恢复" << std::endl;
            std::cout << "3. 数据导出" << std::endl;
            std::cout << "0. 返回主菜单" << std::endl;
            std::cout << "请选择：";
            int subChoice;
            std::cin >> subChoice;
            if (subChoice == 1) {
                std::string path;
                std::cout << "请输入备份目录路径：";
                std::cin >> path;
                backupData(path);
                std::cout << "数据备份成功！" << std::endl;
            }
            else if (subChoice == 2) {
                std::string path;
                std::cout << "请输入备份目录路径：";
                std::cin >> path;
                restoreData(path);
                std::cout << "数据恢复成功！" << std::endl;
            }
            else if (subChoice == 3) {
                std::string path;
                std::cout << "请输入导出目录路径：";
                std::cin >> path;
                exportDataToTxt(path);
                std::cout << "数据导出成功！" << std::endl;
            }
            break;
        }
        default:
            std::cout << "无效的选择，请重新输入！" << std::endl;
            break;
        }
    }
}