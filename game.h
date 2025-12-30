#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include "bird.h"
#include "pipe.h"
#include "questionitem.h"  // 新增
#include "characterselection.h"
#include "main.h"

class Game : public QGraphicsView {
    Q_OBJECT
public:
    Game(QWidget* parent = nullptr);
    void keyPressEvent(QKeyEvent* event);
    void restartGame();
    void setDifficulty(int level);

    // 修改：使用GlobalCharacterConfig
    void setCharacterFromConfig(const GlobalCharacterConfig& config);
    void setCharacter(const CharacterSelection::CharacterInfo& characterInfo);

    // 新增：获取当前管道间隙
    int getCurrentPipeGap() const { return currentPipeGap; }
    void powerOn();   // 每次重新进入游戏时调用

    // 新增：问号道具相关
    void spawnQuestionItem();
    void applyQuestionEffect(QuestionItem::EffectType effect);
    void clearAllQuestionItems();

signals:
    void closed();

private slots:
    void gameLoop();
    void increaseGameDifficulty();
    void updatePhysicsDisplay();
    void loadRecord();
    void saveRecord();
    void shutdownGame();

    // 新增：问号生成定时器
    void onQuestionSpawnTimer();

private:
    QGraphicsScene* scene;
    QGraphicsTextItem* scoreText;
    QGraphicsTextItem* speedText;
    QGraphicsTextItem* difficultyText;
    QGraphicsTextItem* characterText;
    QGraphicsTextItem* recordText;
    QGraphicsTextItem* physicsText;
    QGraphicsTextItem* effectText;  // 新增：效果显示文本
    QElapsedTimer realTime;
    qint64 lastSpeedRaiseMS = 0;
    qint64 lastQuestionSpawnTime = 0;  // 新增：上次生成问号的时间

    bool isPaused = false;
    QGraphicsRectItem* pauseOverlay = nullptr;
    QGraphicsPixmapItem* pausePix = nullptr;

    Bird* bird;
    QTimer* timer;
    QTimer* physicsUpdateTimer;
    QTimer* questionSpawnTimer;  // 新增：问号生成定时器

    QList<Pipe*> pipes;
    QList<QuestionItem*> questionItems;  // 新增：问号道具列表

    int score;
    int record = 0;
    bool isGameOver;
    bool waiting = false;
    qreal globalSpeedFactor;
    int difficultyLevel;
    int pendingDifficulty = 1;

    // 在Game类private部分添加
    bool isInvincible = false;
    QTimer* invincibilityTimer;
    qint64 invincibilityStartTime = 0;
    const int INVINCIBILITY_DURATION = 4000; // 4秒无敌时间
    void activateInvincibility();

    qreal initialGravity;
    qreal initialLift;
    qreal pipeSpeed;
    qreal speedIncrease;
    QString difficultyName;

    int currentPipeGap;
    qreal gravityIncreaseRate;
    qreal liftDecreaseRate;

    QString characterName;
    bool isXiCharacter = false;  // 新增：是否为夕泡泡角色

    GlobalCharacterConfig currentCharacterConfig;

    // 新增：效果持续时间
    QTimer* effectDurationTimer;
    bool isEffectActive = false;
    QString currentEffect;

    // 新增：效果应用方法
    void showEffectMessage(const QString& message);
    void resetEffects();
};

#endif // GAME_H
