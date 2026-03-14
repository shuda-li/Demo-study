#include "Schedule.h"

// 构造函数
Schedule::Schedule() : totalDays(2), slotsPerMorning(10), slotsPerAfternoon(10) {}

Schedule::Schedule(int totalDays, int slotsPerMorning, int slotsPerAfternoon)
    : totalDays(totalDays), slotsPerMorning(slotsPerMorning), slotsPerAfternoon(slotsPerAfternoon) {}

// 场地管理
void Schedule::addVenue(const std::string& venue) {
    venues.push_back(venue);
}

void Schedule::clearVenues() {
    venues.clear();
}

const std::vector<std::string>& Schedule::getVenues() const {
    return venues;
}

// 赛程参数设置
void Schedule::setTotalDays(int days) {
    totalDays = days > 0 ? days : 1;
}

void Schedule::setSlotsPerMorning(int slots) {
    slotsPerMorning = slots > 0 ? slots : 1;
}

void Schedule::setSlotsPerAfternoon(int slots) {
    slotsPerAfternoon = slots > 0 ? slots : 1;
}

// 计算项目权重（参赛人数越多，权重越高，优先安排）
int Schedule::getEventWeight(const Event& event) const {
    return event.getParticipantCount();
}

// 检查运动员时间冲突
bool Schedule::checkAthleteConflict(int eventId, const ScheduleTime& time, const std::vector<Athlete>& athletes, const std::vector<Event>& events) const {
    // 找到当前项目
    const Event* currentEvent = nullptr;
    for (const auto& e : events) {
        if (e.getId() == eventId) {
            currentEvent = &e;
            break;
        }
    }
    if (!currentEvent) return false;

    // 遍历当前项目所有运动员
    for (int athleteId : currentEvent->getParticipantIds()) {
        // 找到运动员
        const Athlete* athlete = nullptr;
        for (const auto& a : athletes) {
            if (a.getId() == athleteId) {
                athlete = &a;
                break;
            }
        }
        if (!athlete) continue;

        // 检查该运动员的其他项目是否在同一时间段
        for (int otherEventId : athlete->getEventIds()) {
            if (otherEventId == eventId) continue;

            // 查找其他项目的赛程
            for (const auto& item : scheduleItems) {
                if (item.eventId == otherEventId) {
                    // 同一时间段冲突
                    if (item.time.isConflict(time)) {
                        return true;
                    }
                    // 连续项目冲突（同一个半天内的相邻slot）
                    if (item.time.day == time.day && item.time.isMorning == time.isMorning) {
                        if (abs(item.time.timeSlot - time.timeSlot) <= 1) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

// 检查场地冲突
bool Schedule::checkVenueConflict(const ScheduleTime& time, const std::string& venue) const {
    for (const auto& item : scheduleItems) {
        if (item.time.isConflict(time) && item.time.venue == venue) {
            return true;
        }
    }
    return false;
}

// 核心：智能生成秩序册（贪心算法+约束校验）
bool Schedule::generateSchedule(const std::vector<Event>& events, const std::vector<Athlete>& athletes) {
    scheduleItems.clear();
    if (venues.empty()) {
        std::cerr << "错误：未设置比赛场地，无法生成秩序册" << std::endl;
        return false;
    }
    if (events.empty()) {
        std::cerr << "错误：无比赛项目，无法生成秩序册" << std::endl;
        return false;
    }

    // 1. 过滤已取消的项目，按权重排序（参赛人数多的优先）
    std::vector<Event> validEvents;
    for (const auto& e : events) {
        if (!e.isEventCancelled()) {
            validEvents.push_back(e);
        }
    }
    std::sort(validEvents.begin(), validEvents.end(), [this](const Event& a, const Event& b) {
        return getEventWeight(a) > getEventWeight(b);
        });

    // 2. 初始化时间槽统计（均衡上下午项目数）
    int totalValidEvents = validEvents.size();
    int totalSlots = totalDays * (slotsPerMorning + slotsPerAfternoon);
    if (totalSlots < totalValidEvents) {
        std::cerr << "错误：比赛时间槽不足，无法安排所有项目" << std::endl;
        return false;
    }

    // 3. 逐个安排项目
    for (const auto& event : validEvents) {
        bool scheduled = false;
        ScheduleTime bestTime;
        std::string bestVenue;

        // 遍历所有可用的时间和场地，找到第一个符合条件的位置
        for (int day = 1; day <= totalDays && !scheduled; day++) {
            // 先上午后下午，均衡安排
            for (int half = 0; half < 2 && !scheduled; half++) {
                bool isMorning = (half == 0);
                int maxSlot = isMorning ? slotsPerMorning : slotsPerAfternoon;

                for (int slot = 1; slot <= maxSlot && !scheduled; slot++) {
                    for (const auto& venue : venues) {
                        ScheduleTime time;
                        time.day = day;
                        time.isMorning = isMorning;
                        time.timeSlot = slot;
                        time.venue = venue;

                        // 检查场地冲突和运动员冲突
                        if (!checkVenueConflict(time, venue) && !checkAthleteConflict(event.getId(), time, athletes, events)) {
                            bestTime = time;
                            bestVenue = venue;
                            scheduled = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!scheduled) {
            std::cerr << "错误：项目【" << event.getName() << "】无法找到合适的赛程安排" << std::endl;
            return false;
        }

        // 添加到赛程
        ScheduleItem item;
        item.eventId = event.getId();
        item.eventName = event.getName();
        item.time = bestTime;
        scheduleItems.push_back(item);
    }

    // 4. 按时间排序赛程
    std::sort(scheduleItems.begin(), scheduleItems.end());

    std::cout << "秩序册生成成功！共安排" << scheduleItems.size() << "个项目" << std::endl;
    return true;
}

// 校验赛程是否合法
bool Schedule::validateSchedule() const {
    for (size_t i = 0; i < scheduleItems.size(); i++) {
        for (size_t j = i + 1; j < scheduleItems.size(); j++) {
            // 同一时间同一场地冲突
            if (scheduleItems[i].time.isConflict(scheduleItems[j].time) && scheduleItems[i].time.venue == scheduleItems[j].time.venue) {
                std::cerr << "赛程冲突：" << scheduleItems[i].eventName << " 和 " << scheduleItems[j].eventName << " 场地时间冲突" << std::endl;
                return false;
            }
        }
    }
    return true;
}

// 赛程查询
const std::vector<Schedule::ScheduleItem>& Schedule::getScheduleItems() const {
    return scheduleItems;
}

std::vector<Schedule::ScheduleItem> Schedule::getScheduleByDay(int day) const {
    std::vector<ScheduleItem> res;
    for (const auto& item : scheduleItems) {
        if (item.time.day == day) {
            res.push_back(item);
        }
    }
    return res;
}

std::vector<Schedule::ScheduleItem> Schedule::getScheduleByEvent(int eventId) const {
    std::vector<ScheduleItem> res;
    for (const auto& item : scheduleItems) {
        if (item.eventId == eventId) {
            res.push_back(item);
        }
    }
    return res;
}

// 保存到文件
bool Schedule::saveToFile(std::ofstream& ofs) const {
    if (!ofs.is_open()) return false;
    // 先保存配置
    ofs << totalDays << "|" << slotsPerMorning << "|" << slotsPerAfternoon << std::endl;
    // 保存场地
    for (const auto& v : venues) ofs << v << "|";
    ofs << std::endl;
    // 保存赛程
    for (const auto& item : scheduleItems) {
        ofs << item.eventId << "|" << item.eventName << "|" << item.time.day << "|"
            << (item.time.isMorning ? 1 : 0) << "|" << item.time.timeSlot << "|" << item.time.venue << std::endl;
    }
    return ofs.good();
}

// 从文件加载
bool Schedule::loadFromFile(std::ifstream& ifs) {
    if (!ifs.is_open()) return false;
    scheduleItems.clear();
    venues.clear();

    std::string line;
    // 加载配置
    if (!std::getline(ifs, line)) return false;
    std::stringstream ss1(line);
    std::string item;
    std::vector<std::string> items;
    while (std::getline(ss1, item, '|')) items.push_back(trim(item));
    if (items.size() < 3) return false;
    int tempDays, tempMorning, tempAfternoon;
    if (!stringToInt(items[0], tempDays)) return false;
    if (!stringToInt(items[1], tempMorning)) return false;
    if (!stringToInt(items[2], tempAfternoon)) return false;
    totalDays = tempDays;
    slotsPerMorning = tempMorning;
    slotsPerAfternoon = tempAfternoon;

    // 加载场地
    if (!std::getline(ifs, line)) return false;
    std::stringstream ss2(line);
    items.clear();
    while (std::getline(ss2, item, '|')) {
        std::string v = trim(item);
        if (!v.empty()) venues.push_back(v);
    }

    // 加载赛程
    while (std::getline(ifs, line)) {
        line = trim(line);
        if (line.empty()) continue;
        std::stringstream ss3(line);
        items.clear();
        while (std::getline(ss3, item, '|')) items.push_back(trim(item));
        if (items.size() < 6) continue;

        ScheduleItem sitem;
        int tempEventId, tempDay, tempIsMorning, tempSlot;
        if (!stringToInt(items[0], tempEventId)) continue;
        if (!stringToInt(items[2], tempDay)) continue;
        if (!stringToInt(items[3], tempIsMorning)) continue;
        if (!stringToInt(items[4], tempSlot)) continue;

        sitem.eventId = tempEventId;
        sitem.eventName = items[1];
        sitem.time.day = tempDay;
        sitem.time.isMorning = (tempIsMorning == 1);
        sitem.time.timeSlot = tempSlot;
        sitem.time.venue = items[5];

        scheduleItems.push_back(sitem);
    }

    std::sort(scheduleItems.begin(), scheduleItems.end());
    return true;
}

// 打印完整秩序册
void Schedule::printFullSchedule() const {
    std::cout << "===================== 运动会秩序册 =====================" << std::endl;
    int currentDay = 0;
    bool currentMorning = false;

    for (const auto& item : scheduleItems) {
        if (item.time.day != currentDay) {
            currentDay = item.time.day;
            std::cout << std::endl << "--------------------- 第" << currentDay << "天 ---------------------" << std::endl;
            currentMorning = !item.time.isMorning; // 强制刷新上午/下午
        }
        if (item.time.isMorning != currentMorning) {
            currentMorning = item.time.isMorning;
            std::cout << std::endl << (currentMorning ? "【上午】" : "【下午】") << std::endl;
            std::cout << std::left << std::setw(10) << "时间段" << std::setw(20) << "项目名称" << std::setw(15) << "比赛场地" << std::endl;
            std::cout << "--------------------------------------------------------" << std::endl;
        }
        std::cout << std::left << std::setw(10) << item.time.timeSlot
            << std::setw(20) << item.eventName
            << std::setw(15) << item.time.venue
            << std::endl;
    }
    std::cout << "========================================================" << std::endl;
}

// 按天打印赛程
void Schedule::printScheduleByDay(int day) const {
    auto dayItems = getScheduleByDay(day);
    if (dayItems.empty()) {
        std::cout << "第" << day << "天无赛程安排" << std::endl;
        return;
    }

    std::cout << "===================== 第" << day << "天赛程 =====================" << std::endl;
    bool currentMorning = false;
    for (const auto& item : dayItems) {
        if (item.time.isMorning != currentMorning) {
            currentMorning = item.time.isMorning;
            std::cout << std::endl << (currentMorning ? "【上午】" : "【下午】") << std::endl;
            std::cout << std::left << std::setw(10) << "时间段" << std::setw(20) << "项目名称" << std::setw(15) << "比赛场地" << std::endl;
            std::cout << "--------------------------------------------------------" << std::endl;
        }
        std::cout << std::left << std::setw(10) << item.time.timeSlot
            << std::setw(20) << item.eventName
            << std::setw(15) << item.time.venue
            << std::endl;
    }
    std::cout << "========================================================" << std::endl;
}

// 导出秩序册到文件
void Schedule::exportScheduleToTxt(const std::string& filePath) const {
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        std::cerr << "无法打开文件：" << filePath << std::endl;
        return;
    }

    ofs << "===================== 学校运动会秩序册 =====================" << std::endl;
    int currentDay = 0;
    bool currentMorning = false;

    for (const auto& item : scheduleItems) {
        if (item.time.day != currentDay) {
            currentDay = item.time.day;
            ofs << std::endl << "--------------------- 第" << currentDay << "天 ---------------------" << std::endl;
            currentMorning = !item.time.isMorning;
        }
        if (item.time.isMorning != currentMorning) {
            currentMorning = item.time.isMorning;
            ofs << std::endl << (currentMorning ? "【上午】" : "【下午】") << std::endl;
            ofs << std::left << std::setw(10) << "时间段" << std::setw(20) << "项目名称" << std::setw(15) << "比赛场地" << std::endl;
            ofs << "--------------------------------------------------------" << std::endl;
        }
        ofs << std::left << std::setw(10) << item.time.timeSlot
            << std::setw(20) << item.eventName
            << std::setw(15) << item.time.venue
            << std::endl;
    }
    ofs.close();
    std::cout << "秩序册已导出到：" << filePath << std::endl;
}