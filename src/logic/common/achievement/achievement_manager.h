#ifndef ACHIEVEMENT_MANAGER_H
#define ACHIEVEMENT_MANAGER_H

#include "achievement.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <lua.hpp>

// 成就管理器
class AchievementManager
{
public:
    AchievementManager();

    void Load_achievements();   // 从配置文件加载成就数据，存储到redis中
    const Achievement* Get_achievement(int achievement_id) const;   // 获取成就数据

    void Init_player_achievements();   // 初始化玩家成就数据
    void Update_achievement(int player_id, const std::string& type, int value);   // 更新成就进度

    bool Is_achievement_complete(int achievement_id);   // 判断成就是否完成
private:
    std::unordered_map<int, Achievement> achievements;   // 成就数据
};


#endif // !ACHIEVEMENT_MANAGER_H


