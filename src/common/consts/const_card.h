#ifndef CONST_CARD_H
#define CONST_CARD_H

// 卡牌相关常量

namespace rpc_server
{

// 卡牌类型枚举
enum CardType
{
    MINION = 0,  // 随从
    SPELL = 1,   // 法术（奥秘，任务）
    WEAPON = 2,  // 武器
    HERO = 3     // 英雄（技能作为英雄的一个函数）
};

// 职业枚举
enum Profession
{
    NEUTRAL = 0,  // 中立
    WARRIOR = 1,  // 战士
    PALADIN = 2,  // 圣骑士
    HUNTER = 3,   // 猎人
    ROGUE = 4,    // 潜行者
    PRIEST = 5,   // 牧师
    SHAMAN = 6,   // 萨满
    MAGE = 7,     // 法师
    WARLOCK = 8,  // 术士
    DRUID = 9,     // 德鲁伊
    DOMON_HUNTER = 10,  // 恶魔猎手
    DEATH_KNIGHT = 11  // 死亡骑士
};

// 卡牌种族枚举
enum RACE
{
    NONE = 0,   // 无种族
    ALL = 1, // 全部
    BEAST = 2,  // 野兽
    DEMON = 3,  // 恶魔
    DRAGON = 4,  // 龙
    ELEMENTAL = 5,  // 元素
    MECH = 6,  // 机械
    MURLOC = 7,  // 鱼人
    PIRATE = 8,  // 海盗
    TOTEM = 9  // 图腾
};

// 卡牌稀有度枚举
enum Rarity
{
    COMMON = 0,  // 普通
    RARE = 1,    // 稀有
    EPIC = 2,    // 史诗
    LEGENDARY = 3  // 传说
};

// 卡牌皮肤枚举
enum Skin
{
    NORMAL = 0,  // 普通
    GOLDEN = 1,  // 金色
    DIAMOND = 2,  // 钻石
    ALTERNATE = 3  // 异画
};

}

#endif // !CONST_CARD_H


