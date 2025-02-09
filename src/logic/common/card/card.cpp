#include "card.h"

// 获取卡牌名称
std::string Card::Get_name() const
{
    return card_name;
}

// 获取卡牌描述
std::string Card::Get_description() const
{
    return card_description;
}

// 获取卡牌类型
CardType Card::Get_type() const
{
    return card_type;
}

// 获取卡牌图片路径
std::string Card::Get_img_dir() const
{
    return card_img_dir;
}

// 获取卡牌费用
int Card::Get_cost() const
{
    return card_cost;
}
