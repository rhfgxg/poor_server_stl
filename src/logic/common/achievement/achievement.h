#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include "common.grpc.pb.h"
#include "consts/const_achievement.h"
// 成就类
class Achievement
{
public:
    int id;
    std::string name;   // 名称
    std::string description;    // 描述
    rpc_server::AchievementType type;   // 类型
    int condition;  // 条件
    std::string reward;  // 奖励

    Achievement(int id, const std::string& name, const std::string& description, const rpc_server::AchievementType& type, int condition, const std::string& reward);
};

#endif // ACHIEVEMENT_H
