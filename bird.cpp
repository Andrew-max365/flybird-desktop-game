#include "main.h"
#include "bird.h"
#include <QGraphicsScene>
#include <QFile>
#include <QDebug>

Bird::Bird(qreal gravity, qreal lift, int size, const QString& imagePath) :
    velocity(0),
    maxDownwardSpeed(15.0),
    targetGravity(gravity),
    targetLift(lift),
    currentSize(size),
    currentImagePath(imagePath) {

    // 应用初始参数
    applyDifficultyParameters();

    // 保存原始值
    originalGravity = this->gravity;
    originalLift = this->lift;

    // 使用参数设置贴图
    setPixmap(QPixmap(currentImagePath).scaled(currentSize, currentSize, Qt::KeepAspectRatio));
    setPos(100, 300);
}

void Bird::flap() {
    velocity = lift;
    // 更新贴图为展翅状态
    setPixmap(QPixmap(currentImagePath).scaled(currentSize, currentSize, Qt::KeepAspectRatio));
}

void Bird::updatePosition() {
    velocity += gravity;

    // 限制最大下落速度
    if (velocity > maxDownwardSpeed) {
        velocity = maxDownwardSpeed;
    }

    setY(y() + velocity);

    // 限制鸟的活动范围
    if (y() < 0) {
        setY(0);
        velocity = 0;
    }
    else if (y() > 560) {
        setY(560);
        velocity = 0;
    }

    // 保持贴图更新
    setPixmap(QPixmap(currentImagePath).scaled(currentSize, currentSize, Qt::KeepAspectRatio));
}

void Bird::setDifficultyParameters(qreal gravity, qreal lift) {
    targetGravity = gravity;
    targetLift = lift;
}

void Bird::applyDifficultyParameters() {
    this->gravity = targetGravity;
    this->lift = targetLift;
}

// 新增：切换角色（使用结构体）
void Bird::changeCharacter(const CharacterSettings& settings) {
    changeCharacter(settings.imagePath, settings.size, settings.gravity, settings.lift);
}

// 新增：切换角色（使用参数）
void Bird::changeCharacter(const QString& imagePath, int size, qreal gravity, qreal lift) {
    // 更新属性
    currentImagePath = imagePath;
    currentSize = size;

    // 更新物理参数
    setDifficultyParameters(gravity, lift);
    applyDifficultyParameters();

    // 更新原始值（新角色的基础值）
    originalGravity = gravity;
    originalLift = lift;

    // 更新贴图
    updateBirdImage(imagePath);
    updateBirdSize(size);

    qDebug() << "角色已切换: 图片=" << imagePath
             << "尺寸=" << size
             << "重力=" << gravity
             << "跳跃力=" << lift;
}

// 新增：更新鸟的大小
void Bird::updateBirdSize(int newSize) {
    currentSize = newSize;

    // 保持当前位置中心点不变
    QPointF centerPos = pos() + boundingRect().center();

    // 重新设置贴图
    QPixmap pixmap(currentImagePath);
    if (!pixmap.isNull()) {
        setPixmap(pixmap.scaled(currentSize, currentSize, Qt::KeepAspectRatio));

        // 调整位置，使中心点保持不变
        setPos(centerPos - boundingRect().center());
    }
}

// 新增：更新鸟的图片
void Bird::updateBirdImage(const QString& imagePath) {
    currentImagePath = imagePath;

    QPixmap pixmap(currentImagePath);
    if (!pixmap.isNull()) {
        setPixmap(pixmap.scaled(currentSize, currentSize, Qt::KeepAspectRatio));
    } else {
        qWarning() << "无法加载鸟图片:" << imagePath;
        // 使用默认图片
        currentImagePath = ":/assets/images/b1.png";
        QPixmap defaultPixmap(currentImagePath);
        if (!defaultPixmap.isNull()) {
            setPixmap(defaultPixmap.scaled(currentSize, currentSize, Qt::KeepAspectRatio));
        }
    }
}

void Bird::increaseDifficulty(qreal gravityRate, qreal liftRate) {
    // 增加重力（下落更快）
    gravity *= gravityRate;

    // 减小上升力（跳跃高度降低）
    lift *= liftRate;

    // 设置上限
    if (gravity > 2.0) gravity = 2.0;
    if (lift > -6.0) lift = -6.0;  // lift是负值，所以绝对值越小越难跳

    // 增加最大下落速度
    maxDownwardSpeed *= 1.02;
    if (maxDownwardSpeed > 25.0) maxDownwardSpeed = 25.0;
}

void Bird::resetDifficulty() {
    gravity = originalGravity;
    lift = originalLift;
    maxDownwardSpeed = 15.0;
}

void Bird::reset() {
    resetDifficulty();
    velocity = 0;

    // 使用当前角色的图片和大小
    setPixmap(QPixmap(currentImagePath).scaled(currentSize, currentSize, Qt::KeepAspectRatio));
    setPos(100, 300);
}
