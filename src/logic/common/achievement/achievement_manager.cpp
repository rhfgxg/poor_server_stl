#include "achievement_manager.h"

AchievementManager::AchievementManager()
{

}

// 从配置文件加载成就数据
void AchievementManager::Load_achievements()
{
    /* 每次启动逻辑服时，从配置文件中记载成就数据，并存储到redis中
    * 在配置文件更新后，调用接口来更新redis中的成就数据
    */
}

// 获取成就数据
const Achievement* AchievementManager::Get_achievement(int achievement_id) const
{
    /* 参数：成就id
    * 返回值：成就配置数据
    */
    auto it = achievements.find(achievement_id);
    if(it != achievements.end()) {
        return &it->second;
    }
    return nullptr;
}

// 初始化玩家成就数据
void AchievementManager::Init_player_achievements()
{
    /* 玩家创建时，初始化玩家的成就数据
    * 也可能移动到逻辑服务器中
    */
}

// 更新成就进度
void AchievementManager::Update_achievement(int player_id, const std::string& type, int value)
{
    /* 更新玩家的成就进度
    * 当玩家的某项数据发生变化时，调用此接口来更新成就进度
    * 参数：成就类型type，变化值value
    * 判断是否达成成就，达成则发送成就奖励
    */ 
}

// 判断成就是否完成
bool AchievementManager::Is_achievement_complete(int achievement_id)
{
    /* 判断玩家是否完成某项成就
    * 参数：成就id
    * 返回值：是否完成
    */
    return false;
}
