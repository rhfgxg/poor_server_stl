#ifndef CARD_H
#define CARD_H

#include "consts/const_card.h"  // 卡牌相关常量

#include <string>

// 卡牌类抽象基类
class Card
{
public:
    virtual Card() = default; 
    virtual ~Card() = default;

    // 获取卡牌名称
    std::string Get_name() const;
    // 获取卡牌描述
    std::string Get_description() const;
    // 获取卡牌类型
    CardType Get_type() const;
    // 获取卡牌图片路径
    std::string Get_img_dir() const;
    // 获取卡牌费用
    int Get_cost() const;

private:
    std::string card_name;  // 卡牌名称
    std::string card_description;   // 卡牌描述
    std::string card_type;  // 卡牌类型
    std::string card_img_dir;   // 卡牌图片路径
    int card_cost;  // 卡牌费用

};

#endif // CARD_H

