#include "card.h"

// 获取卡牌名称
std::string Get_name() const
{
    return card_name;
}

// 获取卡牌描述
std::string Get_description() const
{
    return card_description;
}

// 获取卡牌类型
CardType Get_type() const
{
    return card_type;
}

// 获取卡牌图片路径
std::string Get_img_dir() const
{
    return card_img_dir;
}

// 获取卡牌费用
int Get_cost() const
{
    return card_cost;
}
