#ifndef MAIN_H
#define MAIN_H

#include <QString>  // 添加QString支持

// 游戏速度相关宏定义
#define InitialSpeed 0.7
#define AddSpeedPerTime 3500
#define MaxSpeed 3.0
#define QuestionCreateTime 7000
// 难度相关参数
#define EASY_GRAVITY 0.8
//基础重力
#define EASY_LIFT -10.0
//基础升力
#define EASY_SPEED 5.0
//基础速度
#define EASY_SPEED_INCREASE 0.05
//速度倍率提升速度
#define EASY_GRAVITY_INCREASE_RATE 1.00
// 重力增加率（2%）
#define EASY_LIFT_DECREASE_RATE 1.00
// 升力减少率（1%）
#define EASY_GAP 200;     // 简单：大间隙

#define MEDIUM_GRAVITY 0.98
#define MEDIUM_LIFT -10.0
#define MEDIUM_SPEED 6.0
#define MEDIUM_SPEED_INCREASE 0.1
#define MEDIUM_GRAVITY_INCREASE_RATE 1.01
// 新增：重力增加率（5%）
#define MEDIUM_LIFT_DECREASE_RATE 0.99
// 新增：升力减少率（2%）

#define MEDIUM_GAP 180   // 中等：中等间隙

#define HARD_GRAVITY 1.15
#define HARD_LIFT -10.0
#define HARD_SPEED 8.0
#define HARD_SPEED_INCREASE 0.15
#define HARD_GRAVITY_INCREASE_RATE 1.08
// 新增：重力增加率（8%）
#define HARD_LIFT_DECREASE_RATE 0.96
// 新增：升力减少率（4%）
#define HARD_GAP 160;     // 困难：小间隙

// 新增：全局角色配置结构
struct GlobalCharacterConfig {
    // 默认值为经典小鸟配置
    QString imagePath = ":/assets/images/b1.png";
    int size = 50;
    double gravity = MEDIUM_GRAVITY;  // 使用double替代qreal，因为qreal可能是float或double
    double lift = MEDIUM_LIFT;        // 同上
    QString name = "经典小鸟";

    // 构造函数，提供完整初始化
    GlobalCharacterConfig() {
        // 使用初始化列表已设置默认值
    }

    GlobalCharacterConfig(const QString& imgPath, int sz, double grav, double lft, const QString& nm)
        : imagePath(imgPath), size(sz), gravity(grav), lift(lft), name(nm) {}

    // 重置为默认值
    void resetToDefault() {
        imagePath = ":/assets/images/b1.png";
        size = 50;
        gravity = MEDIUM_GRAVITY;
        lift = MEDIUM_LIFT;
        name = "经典小鸟";
    }

    // 检查配置是否有效
    bool isValid() const {
        return !imagePath.isEmpty() && size > 0;
    }

    // 获取配置描述
    QString toString() const {
        return QString("角色: %1, 尺寸: %2, 重力: %3, 跳跃力: %4")
            .arg(name).arg(size).arg(gravity, 0, 'f', 2).arg(lift, 0, 'f', 1);
    }
};

// 声明全局配置变量（在main.cpp中定义）
extern GlobalCharacterConfig g_characterConfig;

// 全局配置管理函数声明
void initGlobalConfig();
GlobalCharacterConfig& getGlobalCharacterConfig();
void setGlobalCharacterConfig(const GlobalCharacterConfig& config);
void resetGlobalCharacterConfig();

#endif // MAIN_H
