#include "questionitem.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QPainter>      // 新增：用于绘图
#include <QPen>          // 新增：用于设置画笔
#include <QFont>         // 新增：用于设置字体

QuestionItem::QuestionItem(qreal x, qreal y, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent), collected(false), visibleState(true) {

    // 生成随机效果
    generateRandomEffect();

    // 加载问号图片
    QPixmap pixmap(":/assets/images/question.png");
    if (pixmap.isNull()) {
        // 如果图片不存在，创建一个简单的问号
        pixmap = QPixmap(40, 40);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::yellow, 3));
        painter.setBrush(Qt::red);
        painter.drawEllipse(5, 5, 30, 30);
        painter.setPen(QPen(Qt::white, 2));
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.drawText(pixmap.rect(), Qt::AlignCenter, "?");
    }

    setPixmap(pixmap.scaled(60, 60, Qt::KeepAspectRatio));
    setPos(x, y);
    setZValue(10);  // 确保在管道前面

    // 闪烁效果定时器
    flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, this, &QuestionItem::toggleVisibility);
    flashTimer->start(300);  // 每300ms闪烁一次
}

QuestionItem::~QuestionItem() {
    if (flashTimer) {
        flashTimer->stop();
        delete flashTimer;
    }
}

void QuestionItem::generateRandomEffect() {
    int effect = QRandomGenerator::global()->bounded(6);
    effectType = static_cast<EffectType>(effect);

    qDebug() << "生成问号道具，效果类型:" << getEffectDescription();
}

QString QuestionItem::getEffectDescription() const {
    switch (effectType) {
    case SpeedDecrease:    return "速度减小";
    case GapIncrease:      return "管道间隙增加";
    case GravityDecrease:  return "重力减小";
    case LiftIncrease:     return "升力增加";
    case BirdShrink:       return "鸟变小";
    case BirdEnlarge:      return "鸟变大";
    default:               return "未知效果";
    }
}

void QuestionItem::toggleVisibility() {
    visibleState = !visibleState;
    setVisible(visibleState);
}

void QuestionItem::move(qreal speed) {
    if (!collected) {
        setX(x() - speed);
    }
}

void QuestionItem::setCollected(bool value) {
    collected = value;
    if (collected) {
        setVisible(false);
        if (flashTimer) {
            flashTimer->stop();
        }
        emit collectedSignal(effectType);
    }
}
