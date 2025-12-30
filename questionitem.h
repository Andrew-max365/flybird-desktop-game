#ifndef QUESTIONITEM_H
#define QUESTIONITEM_H

#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QObject>

class QuestionItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT  // 必须要有这个
public:
    explicit QuestionItem(qreal x, qreal y, QGraphicsItem* parent = nullptr);
    ~QuestionItem();

    void move(qreal speed);
    bool isCollected() const { return collected; }
    void setCollected(bool value);

    enum EffectType {
        SpeedDecrease,      // 速度减小
        GapIncrease,        // 管道间隙增加
        GravityDecrease,    // 重力减小
        LiftIncrease,       // 升力增加
        BirdShrink,         // 鸟变小
        BirdEnlarge         // 鸟变大
    };

    EffectType getEffectType() const { return effectType; }
    QString getEffectDescription() const;

signals:
    void collectedSignal(QuestionItem::EffectType effect);

private:
    bool collected;
    EffectType effectType;
    QTimer* flashTimer;
    bool visibleState;

private slots:
    void toggleVisibility();
    void generateRandomEffect();
};

#endif // QUESTIONITEM_H
