#include "achievement.h"

Achievement::Achievement(int id, const std::string& name, const std::string& description, const rpc_server::AchievementType& type, int condition, const std::string& reward):
    id(id),
    name(name),
    description(description),
    type(type),
    condition(condition),
    reward(reward)
{

}
