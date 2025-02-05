#ifndef MINION_H
#define MINION_H

// 随从卡牌类
class CardMinion
{
public:
    // 获取攻击力
    int getAttack() const;
    // 获取生命值
    int getHealth() const;

private:
    int attack;  // 攻击力
    int health;  // 生命值
};

#endif // !MINION_H
