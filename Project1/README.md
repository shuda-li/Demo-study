校园运动会管理系统 README

项目功能：
1# 本系统是纯 C++ 开发的控制台版校园运动会管理工具，无数据库依赖，通过本地文件实现数据持久化，覆盖运动会全流程管理需求：
系统设置模块
2# 管理参赛队伍（学院）信息，支持添加、编辑、删除队伍
3# 配置比赛项目信息，设置项目类型（田赛 / 径赛）、性别限制、比赛时长、场地、报名规则（每人最多报 3 项）、计分规则
4# 设置运动会的开始 / 结束时间、报名截止时间

运动员报名模块
    单个运动员报名，自动校验报名规则（性别限制、项目数量上限）
    批量导入运动员报名信息，支持格式校验
    查看、修改、删除运动员报名信息，统计各项目报名人数
    秩序册生成模块
    基于贪心算法自动生成无冲突的赛程安排，规避场地冲突和运动员时间冲突
    手动调整已生成的赛程，支持修改项目的时间、场地
    导出秩序册为文本文件，可直接打印

成绩管理模块
    录入比赛成绩，支持田赛（长度 / 高度）、径赛（时间）两种成绩格式
    修改已录入的成绩，自动重新计算排名和得分
    查询运动员的单项成绩、总得分
    统计查询模块
    多维度查询：按运动员姓名 / 学号、队伍名称、项目名称查询相关信息
    生成学院总分排行榜、运动员个人得分排行榜、项目成绩排名表
    支持线性遍历和二分查找两种查询算法，适配不同数据规模

数据管理模块
    自动备份数据到本地文件，带时间戳标识
    从备份文件恢复数据
    导出所有数据为文本格式
    日志管理模块
    记录所有用户操作日志，支持查看、清空日志


项目结构
校园运动会管理系统/
├── 头文件/
│   ├── Athlete.h       # 运动员数据模型：定义运动员的属性（学号、姓名、性别、所属队伍、参赛项目等）
│   ├── Common.h        # 通用工具：定义全局常量、枚举类型（如项目类型、性别类型）、通用工具函数声明
│   ├── Event.h         # 比赛项目模型：定义项目的属性（项目ID、名称、类型、性别限制、时长、场地、计分规则等）
│   ├── Result.h        # 成绩数据模型：定义成绩的属性（成绩ID、所属项目、所属运动员、成绩数值、排名、得分等）
│   ├── Schedule.h      # 赛程模型+算法声明：定义赛程属性，声明秩序册生成算法的接口
│   ├── SportsManager.h # 系统主控制器：声明系统核心逻辑、查询算法的接口
│   └── Team.h          # 参赛队伍模型：定义队伍的属性（队伍ID、名称、总得分等）
├── 源文件/
│   ├── Athlete.cpp     # 运动员模型实现：实现运动员属性的get/set方法、数据校验逻辑
│   ├── Common.cpp      # 通用工具实现：实现输入验证、时间处理、字符串处理等通用函数
│   ├── Event.cpp       # 比赛项目模型实现：实现项目属性的get/set方法、报名规则校验逻辑
│   ├── main.cpp        # 程序入口：实现控制台主菜单、用户交互逻辑
│   ├── Result.cpp      # 成绩模型+统计算法实现：实现成绩属性的方法，以及成绩排名、得分计算算法
│   ├── Schedule.cpp    # 赛程模型+排程算法实现：实现赛程属性的方法，以及秩序册智能生成算法
│   ├── SportsManager.cpp # 系统主控制器实现：协调各个模块，实现查询算法、数据流转逻辑
│   └── Team.cpp        # 参赛队伍模型实现：实现队伍属性的get/set方法、队伍得分汇总逻辑
└── 资源文件/
    └── README.md       # 项目说明文档

核心算法详解
1. 秩序册智能生成算法（实现文件：Schedule.h/Schedule.cpp）

算法类型
贪心算法，通过优先安排资源占用度高的项目，最小化赛程冲突和运动会总时长。

算法目标
在满足所有约束条件的前提下，生成无冲突的赛程安排，同时尽量缩短运动会整体时长。

约束条件
    同一时间、同一场地只能安排一个比赛项目
    同一运动员的多个参赛项目不能安排在同一时间段
    项目安排时间不能超出运动会预设的总时长范围

算法步骤
    初始化：将所有比赛项目标记为未安排状态，创建空的赛程表，同时初始化场地占用记录表（记录每个场地的已占用时间段）和运动员占用记录表（记录每个运动员的已参赛时间段）。
    项目排序：将所有比赛项目按时长从长到短排序，优先安排时长较长的项目，减少后续排程的冲突概率。

遍历排程：
    对每个待安排项目，从运动会开始时间起，逐时间段查找可用的时间窗口：
    场地冲突检查：检查当前时间段内，项目对应的场地是否未被占用
    运动员冲突检查：检查当前时间段内，该项目的所有参赛运动员均无其他参赛项目
    若找到符合条件的时间段，将项目安排到该时间段，同时更新场地占用表和运动员占用表
    若未找到，将时间窗口向后推进该项目的时长，继续查找下一个可用时间段
    完成排程：重复步骤 3，直到所有项目均完成安排，生成最终的秩序册。

时间复杂度
O (n²)，其中 n 为比赛项目的总数量，适合校园运动会的小规模项目场景（通常项目数量≤50）。


核心代码片段
cpp
// Schedule.cpp 核心实现
void Schedule::generateSchedule(const QList<Event>& events, const QList<Athlete>& athletes, const QDateTime& meetingStartTime, const QDateTime& meetingEndTime) {
    this->schedules.clear();
    QMap<QString, QList<QPair<QDateTime, QDateTime>>> venueOccupied; // 场地占用记录：key=场地名称，value=已占用时间段列表
    QMap<QString, QList<QPair<QDateTime, QDateTime>>> athleteOccupied; // 运动员占用记录：key=运动员学号，value=已参赛时间段列表

    // 步骤2：按项目时长降序排序
    QList<Event> sortedEvents = events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const Event& a, const Event& b) {
        return a.getDuration() > b.getDuration();
    });

    // 步骤3：遍历排程
    for (const Event& event : sortedEvents) {
        QDateTime currentTime = meetingStartTime;
        bool isScheduled = false;
        while (currentTime.addSecs(event.getDuration() * 60) <= meetingEndTime) {
            QDateTime slotEnd = currentTime.addSecs(event.getDuration() * 60);
            bool venueAvailable = true;
            bool athleteAvailable = true;

            // 场地冲突检查
            if (venueOccupied.contains(event.getVenue())) {
                for (const auto& slot : venueOccupied[event.getVenue()]) {
                    if (isTimeOverlap(currentTime, slotEnd, slot.first, slot.second)) {
                        venueAvailable = false;
                        break;
                    }
                }
            }
            if (!venueAvailable) {
                currentTime = slotEnd;
                continue;
            }

            // 运动员冲突检查
            for (const QString& athleteId : event.getParticipantIds()) {
                if (athleteOccupied.contains(athleteId)) {
                    for (const auto& slot : athleteOccupied[athleteId]) {
                        if (isTimeOverlap(currentTime, slotEnd, slot.first, slot.second)) {
                            athleteAvailable = false;
                            break;
                        }
                    }
                    if (!athleteAvailable) break;
                }
            }
            if (athleteAvailable) {
                // 安排项目到当前时间段
                ScheduleItem item;
                item.setEventId(event.getId());
                item.setStartTime(currentTime);
                item.setEndTime(slotEnd);
                item.setVenue(event.getVenue());
                this->schedules.append(item);

                // 更新占用记录
                venueOccupied[event.getVenue()].append(qMakePair(currentTime, slotEnd));
                for (const QString& athleteId : event.getParticipantIds()) {
                    athleteOccupied[athleteId].append(qMakePair(currentTime, slotEnd));
                }
                isScheduled = true;
                break;
            }
            currentTime = slotEnd;
        }
        if (!isScheduled) {
            // 若无法在运动会时长内安排，标记为取消项目
            qWarning() << "项目" << event.getName() << "无法安排，已取消";
        }
    }
}

// 时间冲突检查工具函数
bool Schedule::isTimeOverlap(const QDateTime& start1, const QDateTime& end1, const QDateTime& start2, const QDateTime& end2) {
    return !(end1 <= start2 || start1 >= end2);
}
2. 成绩统计与排名算法（实现文件：Result.h/Result.cpp）
算法目标
根据运动员的比赛成绩自动生成排名，同时计算运动员个人得分和所属队伍的总分。

算法步骤
项目分组：将所有成绩按比赛项目进行分组。

成绩排序：对每个项目的成绩进行针对性排序：

径赛项目：按成绩时间升序排序，时间越短排名越靠前

田赛项目：按成绩数值降序排序，成绩越高排名越靠前

排名计算：
    初始化排名为 1，遍历排序后的成绩列表
    若当前运动员成绩与上一名运动员成绩相同，则给予相同排名
    若成绩不同，则将排名更新为当前遍历的索引 + 1

得分计算：根据项目预设的计分规则（如前 5 名分别获得 5、4、3、2、1 分），为每个运动员计算单项得分。

队伍总分汇总：将每个运动员的得分汇总到其所属队伍，生成队伍总分排行榜。

时间复杂度
    O (m*n log n)，其中 m 为比赛项目数量，n 为每个项目的参赛运动员数量。

核心代码片段
cpp
// Result.cpp 核心实现
void Result::calculateRankAndScore(QList<Result>& results, const QList<Event>& events, QMap<QString, Team>& teams) {
    // 按项目分组
    QMap<QString, QList<Result*>> eventResultMap;
    for (auto& result : results) {
        eventResultMap[result.getEventId()].append(&result);
    }

    // 遍历每个项目计算排名和得分
    for (const auto& event : events) {
        if (!eventResultMap.contains(event.getId())) continue;
        auto& eventResults = eventResultMap[event.getId()];

        // 步骤2：按项目类型排序
        std::sort(eventResults.begin(), eventResults.end(), [&](Result* a, Result* b) {
            if (event.getType() == EventType::Track) {
                // 径赛：时间越短排名越前
                return a->getResultValue() < b->getResultValue();
            } else {
                // 田赛：成绩越高排名越前
                return a->getResultValue() > b->getResultValue();
            }
        });

        // 步骤3：计算排名
        int rank = 1;
        for (int i = 0; i < eventResults.size(); ++i) {
            if (i > 0) {
                bool isSameScore = false;
                if (event.getType() == EventType::Track) {
                    isSameScore = (eventResults[i]->getResultValue() == eventResults[i-1]->getResultValue());
                } else {
                    isSameScore = (eventResults[i]->getResultValue() == eventResults[i-1]->getResultValue());
                }
                if (!isSameScore) {
                    rank = i + 1;
                }
            }
            eventResults[i]->setRank(rank);

            // 步骤4：计算得分
            int score = 0;
            const QMap<int, int>& scoreRule = event.getScoreRule();
            if (scoreRule.contains(rank)) {
                score = scoreRule[rank];
            }
            eventResults[i]->setScore(score);

            // 步骤5：更新队伍总分
            QString teamId = eventResults[i]->getAthlete()->getTeamId();
            if (teams.contains(teamId)) {
                teams[teamId].addTotalScore(score);
            }
        }
    }
}
3. 多维度查询算法（实现文件：SportsManager.h/SportsManager.cpp）
算法实现
实现两种查询算法，适配不同数据规模的查询需求：

线性遍历查询
    逻辑：遍历所有目标数据，逐一匹配查询条件
    时间复杂度：O (n)，n 为数据总数量
    适用场景：小规模数据查询（如运动员数量≤100），实现简单，无需预处理

二分查找查询
    逻辑：先将数据按唯一标识（如运动员学号、队伍 ID）排序，再使用二分查找快速定位目标数据
    时间复杂度：O (log n)，n 为数据总数量
    适用场景：大规模数据查询（如运动员数量≥100），查询效率更高

支持的查询维度
    运动员查询：按学号（精确匹配）、姓名（模糊匹配）查询运动员的参赛信息、比赛成绩
    队伍查询：按队伍名称（模糊匹配）查询队伍的参赛项目、总得分、运动员列表
    项目查询：按项目名称（模糊匹配）查询项目的参赛运动员、成绩排名、赛程安排


核心代码片段
cpp
// SportsManager.cpp 核心实现
// 线性遍历查询：按运动员姓名模糊匹配
QList<Athlete> SportsManager::searchAthletesByNameLinear(const QString& name) const {
    QList<Athlete> result;
    for (const auto& athlete : this->athletes) {
        if (athlete.getName().contains(name)) {
            result.append(athlete);
        }
    }
    return result;
}

// 二分查找查询：按运动员学号精确匹配
Athlete SportsManager::searchAthleteByIdBinary(const QString& id) const {
    // 先按学号排序
    QList<Athlete> sortedAthletes = this->athletes;
    std::sort(sortedAthletes.begin(), sortedAthletes.end(), [](const Athlete& a, const Athlete& b) {
        return a.getId() < b.getId();
    });

    // 二分查找
    int left = 0;
    int right = sortedAthletes.size() - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (sortedAthletes[mid].getId() == id) {
            return sortedAthletes[mid];
        } else if (sortedAthletes[mid].getId() < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return Athlete(); // 返回空对象表示未找到
}

// 项目查询：按项目名称模糊匹配
QList<Event> SportsManager::searchEventsByName(const QString& name) const {
    QList<Event> result;
    for (const auto& event : this->events) {
        if (event.getName().contains(name)) {
            result.append(event);
        }
    }
    return result;
}

<<<<<!!!!!!!注意事项!!!!!!!>>>>>
<<<<<!!!!!!!注意事项!!!!!!!>>>>>
<<<<<!!!!!!!注意事项!!!!!!!>>>>>

数据录入顺序要求:

    必须按「系统设置→运动员报名→秩序册生成→成绩录入」的顺序操作，否则会因信息不完整导致录入失败或检索错误：

    需先设置参赛队伍和比赛项目，才能进行运动员报名

    需完成运动员报名，才能生成秩序册

    需生成秩序册并完成比赛，才能录入成绩



输入格式要求:
    运动员学号需为纯数字或字母 + 数字组合，长度不超过 10 位

    成绩录入格式：径赛需输入数字格式的时间（如 "12.34"，单位：秒），田赛需输入数字格式的长度 / 高度（如 "2.05"，单位：米）
    
    所有输入的文本信息（姓名、队伍名称、项目名称）不能包含特殊字符（如 /、\、:、*、?、"、<、>、|），否则会导致数据存储失败


报名规则限制:
    每个运动员最多报名 X 个(X自定义）项目，且不能报名不符合性别限制的项目，系统会自动拦截违规报名

    每个项目报名人数不足 4 人时，系统会自动取消该项目，并通知所有报名该项目的运动员

    报名截止时间后，无法再修改、添加、删除运动员报名信息

数据备份与保护：
    系统会在程序正常退出时自动备份数据到backup目录，文件名为带时间戳的格式，请勿手动删除该目录下的文件

    如需手动备份，请复制data目录下的所有文件到其他位置，请勿修改数据文件的格式，否则会导致程序无法读取数据

    若数据文件损坏，可通过「数据管理」模块的「从备份恢复数据」功能恢复最近的备份

成绩录入注意事项：
    成绩录入需在对应项目比赛结束后进行，录入后会自动覆盖原有的成绩（若存在）
    
    同一运动员同一项目只能录入一次成绩，重复录入会提示错误
    
    成绩录入后，系统会自动重新计算该项目的排名和所有相关运动员、队伍的得分，无需手动操作


程序操作要求：
    请勿强制关闭程序（如直接关闭控制台窗口、强制结束进程），需通过主菜单的「退出系统」选项正常退出，否则会导致数据丢失或损坏
    
    操作过程中如提示输入错误，请按照提示重新输入，请勿随意输入无效内容，否则可能导致程序异常
    
    若程序出现异常退出，可重新运行程序，系统会自动检测并修复损坏的数据

秩序册修改注意事项：

    手动调整赛程时，需确保修改后的时间不会与其他项目产生场地冲突或运动员时间冲突，系统会自动校验冲突，若存在冲突会提示无法修改
    
    修改赛程后，需重新导出秩序册以更新内容，否则导出的仍是修改前的秩序册


本项目完全符合需求：覆盖了所有要求的功能，包括系统设置、报名限制、积分规则、秩序册智能生成、成绩统计、数据备份等


再次声明：这个系统的输入要求严格，所有数据都必须符合指定格式，否则会导致程序错误或异常退出。##必须按照系统提示的顺序##输入数据，不能随机输入。
比如说，没有设置参赛单位，但是给运动员报名了，系统会提示错误。因为识别到运动员报名了，但是没有设置参赛单位，所以系统无法判断运动员所属的单位。