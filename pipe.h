// pipe.h - 添加生成问号的方法
#ifndef PIPE_H
#define PIPE_H

#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>

// 前向声明
class QuestionItem;

class Pipe : public QGraphicsItemGroup {
public:
    Pipe(qreal speed = 5.0, int difficultyLevel = 1);
    ~Pipe();

    void setDifficulty(int level);
    void movePipe();
    void setSpeed(qreal newSpeed);
    qreal getSpeed() const;
    void setGlobalSpeedFactor(qreal factor);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // 新增：问号相关方法
    void attachQuestionItem(QuestionItem* question);
    QuestionItem* getQuestionItem() const { return questionItem; }
    void detachQuestionItem();
    qreal getGapCenterY() const;
    bool hasQuestionItem() const { return questionItem != nullptr; }

public:
    bool isPassed;

private:
    QGraphicsPixmapItem* topPipe;
    QGraphicsPixmapItem* bottomPipe;
    QuestionItem* questionItem;  // 新增：关联的问号道具

    qreal speed;
    qreal globalSpeedFactor;
    int gap;
    int difficultyLevel;
    int topHeight;
    int bottomHeight;

    void updateGapByDifficulty();
};

#endif // PIPE_H
