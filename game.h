#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "bird.h"
#include "pipe.h"
#include <QGraphicsTextItem>
#include <QEventLoop>

class Game : public QGraphicsView {
    Q_OBJECT
public:
    Game(QWidget* parent = nullptr);
    void keyPressEvent(QKeyEvent* event);
    void restartGame();
    void loadRecord();     // 加载历史最高分的函数
    void saveRecord();     // 保存历史最高分的函数

private slots:
    void gameLoop();
    void updateTotalTime();  // 更新总时间的函数

private:
    QGraphicsScene* scene;
    QGraphicsTextItem* scoreText;
    QGraphicsTextItem* recordText;     // 记录历史最高分的文本项
    QGraphicsRectItem* pauseOverlay;   // 灰度遮罩
    QGraphicsPixmapItem* pauseTextPix; // 提示图
    Bird* bird;
    QTimer* timer;           // 游戏循环计时器
    QTimer* totalTimeTimer;  // 总时间计时器
    QList<Pipe*> pipes;
    int score;
    int record;              // 历史最高分
    double totalTime;        // 总时间
    bool isGameOver;
    bool isPaused;
    bool waiting = false;   // 1秒 “冷却”
};

#endif // GAME_H

