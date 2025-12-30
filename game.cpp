#include "game.h"
#include <QKeyEvent>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QSettings>

Game::Game(QWidget* parent) : QGraphicsView(parent), pauseOverlay(nullptr),pauseTextPix(nullptr),score(0),record(0),totalTime(0),isGameOver(false),isPaused(false) {
    scene = new QGraphicsScene(this);
    setScene(scene);

    setWindowTitle("Ikun牌小鸟");

    QIcon icon(":/assets/images/bluebird-upflap.png");
    setWindowIcon(icon);

    bird = new Bird();
    scene->addItem(bird);

    // 定时器，控制游戏循环
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::gameLoop);
    timer->start(20);

    // 计时器，控制游戏速度
    totalTimeTimer = new QTimer(this);
    connect(totalTimeTimer, &QTimer::timeout, this, &Game::updateTotalTime);
    totalTimeTimer->start(1000);

    //设置游戏窗口大小
    setFixedSize(400, 600);
    scene->setSceneRect(0, 0, 400, 600);

    scene->setBackgroundBrush(QBrush(QImage(":/assets/images/background-day.png").scaled(400, 600)));

    // 取消滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 创建并显示分数文本项
    scoreText = new QGraphicsTextItem(QString("Score: %1").arg(score));
    //放在最前面
    scoreText->setZValue(1);
    scoreText->setDefaultTextColor(Qt::white);
    scoreText->setFont(QFont("Arial", 20));
    scoreText->setPos(10, 10);
    scene->addItem(scoreText);

    // 创建并显示记录文本项
    recordText = new QGraphicsTextItem(QString("Record: %1").arg(record));
    recordText->setZValue(1);
    recordText->setDefaultTextColor(Qt::white);
    recordText->setFont(QFont("Arial", 20));
    recordText->setPos(10, 50);  // 右上角位置
    scene->addItem(recordText);

    loadRecord();
}

void Game::keyPressEvent(QKeyEvent* event) {
    if (waiting) {               // 冷却期内任何键都无视
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape && !isGameOver) {          // 按 ESC 切换暂停,但是如果游戏结束了就没必要存在暂停逻辑
        isPaused = !isPaused;
        if (isPaused) {
            timer->stop();                         // 停掉 20 ms 的游戏循环计时器
            totalTimeTimer->stop();

            pauseOverlay = new QGraphicsRectItem(scene->sceneRect());
            pauseOverlay->setBrush(QBrush(QColor(0, 0, 0, 127))); // 127 ≈ 50 %
            pauseOverlay->setZValue(1000);                       // 保证在最上层
            scene->addItem(pauseOverlay);


            QPixmap pix(":/assets/images/escloop.png");
            //pix = pix.scaled(300, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation); // 大小自己调
            pauseTextPix = new QGraphicsPixmapItem(pix);
            pauseTextPix->setZValue(1001); // 比灰层再高一点
            pauseTextPix->setPos(
                (width()  - pauseTextPix->boundingRect().width())  / 2,
                (height() - pauseTextPix->boundingRect().height()) / 2);
            scene->addItem(pauseTextPix);

        } else {
            timer->start(20);                      // 恢复
            totalTimeTimer->start(1000);

            if (pauseOverlay) {
                scene->removeItem(pauseOverlay);
                delete pauseOverlay;
                pauseOverlay = nullptr;
            }
            if (pauseTextPix) {
                scene->removeItem(pauseTextPix);
                delete pauseTextPix;
                pauseTextPix = nullptr;
            }
        }
        event->accept();                           // 告诉 Qt 事件已处理
        return;
    }
    if (event->key() == Qt::Key_Space && !isPaused) {
        if (isGameOver) {
            restartGame();  // 如果游戏结束，按空格键重置游戏
        }
        else {
            bird->flap();  // 如果游戏在进行，按空格键让小鸟跳跃
        }
    }
}

void Game::restartGame()
{
    // 清除场景中的管道和文本
    for (Pipe* pipe : pipes) {
        scene->removeItem(pipe);
        delete pipe;
    }
    pipes.clear();

    // 重置小鸟的位置和状态
    bird->setPos(100, 300);
    bird->reset();

    // 重置分数
    score = 0;
    scoreText->setPlainText(QString("Score: %1").arg(score));

    // 移除 Game Over 画面
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item))
        {
            if (pixmapItem->pixmap().cacheKey() == QPixmap(":/assets/images/gameover.png").cacheKey())
            {
                scene->removeItem(pixmapItem);
                delete pixmapItem;
            }
        }
        if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            if (textItem->toPlainText() == "按空格键重新开始") {
                scene->removeItem(textItem);
                delete textItem;
            }
        }
    }

    totalTime = 0;  //初始化游戏时间

    // 重置游戏状态
    isGameOver = false;
    timer->start(20);
    totalTimeTimer->start(1000);

}

void Game::gameLoop() {
    if (isPaused || isGameOver) return;   // 暂停或结束直接退出

    bird->updatePosition();

    // 生成新的管道
    if (pipes.isEmpty() || pipes.last()->x() < 200) {
        Pipe* pipe = new Pipe();
        pipes.append(pipe);
        scene->addItem(pipe);
    }

    // 管道移动与检测碰撞
    auto it = pipes.begin();
    while (it != pipes.end()) {
        Pipe* pipe = *it;
        pipe->movePipe(totalTime);

        // 检测与小鸟的碰撞
        if (bird->collidesWithItem(pipe)) {
            timer->stop();
            QGraphicsPixmapItem* gameOverItem = scene->addPixmap(QPixmap(":/assets/images/gameover.png"));
            // 将 Game Over 画面放在中间位置
            gameOverItem->setPos(this->width() / 2 - gameOverItem->pixmap().width() / 2, this->height() / 2 - gameOverItem->pixmap().height() / 2);
            isGameOver = true;

            if (score > record) {
                record = score;
                saveRecord();
                recordText->setPlainText(QString("Record: %1").arg(record));
            }

            //提示按空格重新游戏，用QGraphicsTextItem
            QGraphicsTextItem* restartText = new QGraphicsTextItem("按空格键重新开始");
            restartText->setDefaultTextColor(Qt::black);
            restartText->setFont(QFont("Arial", 12, QFont::Bold));
            //放在中间
            restartText->setPos(this->width() / 2 - restartText->boundingRect().width() / 2, this->height() / 2 + gameOverItem->pixmap().height() / 2 + 10);
            scene->addItem(restartText);

            waiting = true;                       // 开始冷却
            QTimer::singleShot(1000, this, [this](){ waiting = false; });
            return;
        }
        // 如果小鸟通过了管道（即小鸟的x坐标刚好超过管道的x坐标）
        if (pipe->x() + pipe->boundingRect().width() < bird->x() && !pipe->isPassed) {
            // 增加分数
            score++;
            pipe->isPassed = true;  // 确保不会重复加分

            // 更新分数显示
            scoreText->setPlainText(QString("Score: %1").arg(score));
        }

        // 如果管道移出了屏幕，将其从场景和列表中删除
        if (pipe->x() < -60) {
            scene->removeItem(pipe);
            delete pipe;
            it = pipes.erase(it);  // 从列表中安全移除管道
        }
        else {
            ++it;  // 继续遍历
        }
    }
}

void Game::loadRecord() {
    QSettings settings("GCG", "FlyBird");
    record = settings.value("record", 0).toInt();
    recordText->setPlainText(QString("Record: %1").arg(record));
}

void Game::saveRecord() {
    QSettings settings("GCG", "FlyBird");
    settings.setValue("record", record);
}

void Game::updateTotalTime() {
    totalTime += 1.0;  // 每秒增加1
}
