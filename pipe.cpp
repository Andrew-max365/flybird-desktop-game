// pipe.cpp
#include "main.h"
#include "pipe.h"
#include "questionitem.h"
#include <QRandomGenerator>
#include <QGraphicsScene>
#include <QDebug>

// 难度到间隙的映射常量
Pipe::Pipe(double speed, int difficultyLevel)
    : speed(speed),
    globalSpeedFactor(InitialSpeed),
    difficultyLevel(difficultyLevel),
    isPassed(false) ,questionItem(nullptr){

    // 根据难度级别设置间隙
    updateGapByDifficulty();

    // 随机生成管道高度（基于间隙调整范围）
    int minTopHeight = 100;
    int maxTopHeight = 250;

    // 困难模式下调整高度范围，增加难度
    if (difficultyLevel >= 2) {
        minTopHeight = 120;
        maxTopHeight = 280;
    }

    topHeight = QRandomGenerator::global()->bounded(minTopHeight, maxTopHeight);
    bottomHeight = 600 - topHeight - gap;

    // 检查高度是否合理
    if (topHeight < 50 || bottomHeight < 50) {
        // 如果某个管道太短，重新调整
        topHeight = QRandomGenerator::global()->bounded(150, 250);
        bottomHeight = 600 - topHeight - gap;
    }

    qDebug() << "创建管道: 难度" << difficultyLevel
             << ", 间隙" << gap
             << ", 上管道高度" << topHeight
             << ", 下管道高度" << bottomHeight;

    // 加载并缩放下管道图片
    bottomPipe = new QGraphicsPixmapItem(
        QPixmap(":/assets/images/pipe-green.png").scaled(50, bottomHeight, Qt::IgnoreAspectRatio)
        );

    // 加载上管道图片，并进行180度旋转
    QPixmap pixmap(":/assets/images/pipe-green.png");
    QTransform transform;
    transform.translate(pixmap.width() / 2, pixmap.height() / 2);
    transform.rotate(180);
    transform.translate(-pixmap.width() / 2, -pixmap.height() / 2);
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    // 将旋转后的图片缩放并作为上管道
    topPipe = new QGraphicsPixmapItem(rotatedPixmap.scaled(50, topHeight, Qt::IgnoreAspectRatio));

    // 将上、下管道加入到该组中
    addToGroup(topPipe);
    addToGroup(bottomPipe);

    // 设置上管道和下管道的位置
    topPipe->setPos(0, 0);
    bottomPipe->setPos(0, topHeight + gap);

    // 将整个组移动到屏幕的右侧开始位置
    setPos(400, 0);
}

Pipe::~Pipe() {
    questionItem = nullptr;
}

qreal Pipe::getGapCenterY() const {
    return topHeight + gap / 2.0;
}

void Pipe::attachQuestionItem(QuestionItem* question) {
    if (question && !questionItem) {
        questionItem = question;

        // 计算问号在管道间隙中心的位置
        qreal questionX = x() + 25;  // 管道宽度50，居中
        qreal questionY = y() + topHeight + gap / 2.0 - 20;  // 问号高度40，居中

        question->setPos(questionX, questionY);
        scene()->addItem(question);

        qDebug() << "在管道间隙中心添加问号道具";
    }
}

void Pipe::detachQuestionItem() {
    if (questionItem) {
        scene()->removeItem(questionItem);
        questionItem = nullptr;
    }
}

void Pipe::movePipe() {
    setX(x() - speed * globalSpeedFactor);

    // 移动关联的问号
    if (questionItem && !questionItem->isCollected()) {
        questionItem->move(speed * globalSpeedFactor);
    }
}

void Pipe::updateGapByDifficulty() {
    switch (difficultyLevel) {
    case 0:  // 简单
        gap = EASY_GAP;
        break;
    case 1:  // 中等
        gap = MEDIUM_GAP;
        break;
    case 2:  // 困难
        gap = HARD_GAP;
        break;
    default: // 默认中等
        gap = MEDIUM_GAP;
        difficultyLevel = 1;
    }

    qDebug() << "设置管道间隙为" << gap << " (难度级别: " << difficultyLevel << ")";
}

void Pipe::setDifficulty(int level) {
    if (level < 0 || level > 2) return;

    difficultyLevel = level;
    updateGapByDifficulty();

    // 重新计算管道高度
    int topHeight = QRandomGenerator::global()->bounded(100, 250);
    int bottomHeight = 600 - topHeight - gap;

    // 更新管道尺寸
    bottomPipe->setPixmap(
        QPixmap(":/assets/images/pipe-green.png").scaled(50, bottomHeight, Qt::IgnoreAspectRatio)
        );

    QPixmap pixmap(":/assets/images/pipe-green.png");
    QTransform transform;
    transform.translate(pixmap.width() / 2, pixmap.height() / 2);
    transform.rotate(180);
    transform.translate(-pixmap.width() / 2, -pixmap.height() / 2);
    QPixmap rotatedPixmap = pixmap.transformed(transform);

    topPipe->setPixmap(rotatedPixmap.scaled(50, topHeight, Qt::IgnoreAspectRatio));

    // 更新下管道位置
    bottomPipe->setPos(0, topHeight + gap);
}

// 其他方法保持不变...
void Pipe::setSpeed(qreal newSpeed) {
    speed = newSpeed;
}

qreal Pipe::getSpeed() const {
    return speed;
}

void Pipe::setGlobalSpeedFactor(qreal factor) {
    globalSpeedFactor = factor;
}

QRectF Pipe::boundingRect() const {
    QRectF topRect = topPipe->boundingRect().translated(topPipe->pos());
    QRectF bottomRect = bottomPipe->boundingRect().translated(bottomPipe->pos());
    return topRect.united(bottomRect);
}

QPainterPath Pipe::shape() const {
    QPainterPath path;
    path.addRect(topPipe->boundingRect().translated(topPipe->pos()));
    path.addRect(bottomPipe->boundingRect().translated(bottomPipe->pos()));
    return path;
}
