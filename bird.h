#ifndef BIRD_H
#define BIRD_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QString>

class Bird : public QGraphicsPixmapItem {
public:
    struct CharacterSettings {
        QString imagePath;
        int size;
        qreal gravity;
        qreal lift;
    };

    Bird(qreal gravity = 0.98, qreal lift = -10.0, int size = 120,
         const QString& imagePath = ":/assets/images/bluebird-upflap.png");

    // 物理操作
    void flap();
    void updatePosition();
    void reset();
    void increaseDifficulty(qreal gravityRate = 1.05, qreal liftRate = 0.98);
    void resetDifficulty();
    void setDifficultyParameters(qreal gravity, qreal lift);
    void applyDifficultyParameters();

    // 角色相关
    void changeCharacter(const CharacterSettings& settings);
    void changeCharacter(const QString& imagePath, int size, qreal gravity, qreal lift);
    void updateBirdSize(int newSize);
    void updateBirdImage(const QString& imagePath);

    // 新增：获取物理参数的方法
    qreal getGravity() const { return gravity; }
    qreal getLift() const { return lift; }
    qreal getVelocity() const { return velocity; }
    qreal getMaxDownwardSpeed() const { return maxDownwardSpeed; }

    // 获取当前设置
    int getBirdSize() const { return currentSize; }
    QString getBirdImagePath() const { return currentImagePath; }

private:
    // 物理参数
    qreal velocity;
    qreal gravity;
    qreal originalGravity;
    qreal lift;
    qreal originalLift;
    qreal maxDownwardSpeed;
    qreal targetGravity;
    qreal targetLift;

    // 角色相关属性
    int currentSize;
    QString currentImagePath;
};

#endif // BIRD_H
