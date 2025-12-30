// game.cpp (修复版)

#include "main.h"
#include "game.h"
#include <QKeyEvent>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QDebug>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QSettings>

// 改为extern声明（如果main.h中没有extern声明，可以在这里添加）
extern GlobalCharacterConfig g_characterConfig;
extern void initGlobalConfig();
extern GlobalCharacterConfig& getGlobalCharacterConfig();
extern void setGlobalCharacterConfig(const GlobalCharacterConfig& config);
extern void resetGlobalCharacterConfig();

Game::Game(QWidget* parent)
    : QGraphicsView(parent),
    physicsText(nullptr),
    effectText(nullptr),
    bird(nullptr),
    score(0),
    isGameOver(false),
    globalSpeedFactor(InitialSpeed),
    difficultyLevel(0),
    currentPipeGap(MEDIUM_GAP),
    isXiCharacter(false) // 注意：currentEffect 不能在这里初始化，因为它不是基本类型
{
    // 在构造函数体内初始化 currentEffect
    currentEffect = "";
    scene = new QGraphicsScene(this);
    setScene(scene);
    setWindowTitle("HUST牌小鸟");
    QIcon icon(":/assets/images/b1.png");
    setWindowIcon(icon);

    // 初始化全局配置
    initGlobalConfig();
    loadRecord();

    // 初始化默认难度（中等）
    difficultyName = "普通模式";
    initialGravity = MEDIUM_GRAVITY;
    initialLift = MEDIUM_LIFT;
    pipeSpeed = MEDIUM_SPEED;
    speedIncrease = MEDIUM_SPEED_INCREASE;
    gravityIncreaseRate = MEDIUM_GRAVITY_INCREASE_RATE;
    liftDecreaseRate = MEDIUM_LIFT_DECREASE_RATE;

    // 从全局配置获取角色设置
    GlobalCharacterConfig config = getGlobalCharacterConfig();
    characterName = config.name;
    isXiCharacter = (characterName == "夕泡泡");

    // 创建并显示分数文本项
    scoreText = new QGraphicsTextItem(QString("得分: %1").arg(score));
    scoreText->setZValue(1);
    scoreText->setDefaultTextColor(Qt::white);
    scoreText->setFont(QFont("Arial", 16));
    scoreText->setPos(10, 10);
    scene->addItem(scoreText);

    // 创建并显示历史最高分文本项
    recordText = new QGraphicsTextItem(QString("最高得分: %1").arg(record));
    recordText->setZValue(1);
    recordText->setDefaultTextColor(Qt::white);
    recordText->setFont(QFont("Arial", 16));
    recordText->setPos(120, 10); // 紧挨在分数下面
    scene->addItem(recordText);

    // 创建并显示速度文本项
    speedText = new QGraphicsTextItem(QString("Speed: x%1").arg(globalSpeedFactor, 0, 'f', 1));
    speedText->setZValue(1);
    speedText->setDefaultTextColor(Qt::yellow);
    speedText->setFont(QFont("Arial", 14));
    speedText->setPos(10, 40);
    scene->addItem(speedText);

    // 创建并显示难度文本项
    difficultyText = new QGraphicsTextItem(QString("模式: %1").arg(difficultyName));
    difficultyText->setZValue(1);
    difficultyText->setDefaultTextColor(Qt::cyan);
    difficultyText->setFont(QFont("Arial", 12));
    difficultyText->setPos(10, 70);
    scene->addItem(difficultyText);

    // 创建并显示角色文本项
    characterText = new QGraphicsTextItem(QString("角色: %1").arg(characterName));
    characterText->setZValue(1);
    characterText->setDefaultTextColor(Qt::magenta);
    characterText->setFont(QFont("Arial", 12));
    characterText->setPos(10, 100);
    scene->addItem(characterText);

    // 5. 新增：物理数据显示
    physicsText = new QGraphicsTextItem("物理: 重力: -- 升力: -- 速度: --");
    physicsText->setZValue(1);
    physicsText->setDefaultTextColor(QColor(255, 200, 100)); // 橙色
    physicsText->setFont(QFont("Arial", 11));
    physicsText->setPos(10, 130); // 放在角色显示下面
    scene->addItem(physicsText);

    // 新增：效果显示文本
    effectText = new QGraphicsTextItem("");
    effectText->setZValue(100);
    effectText->setDefaultTextColor(Qt::yellow);
    effectText->setFont(QFont("Arial", 14, QFont::Bold));
    effectText->setPos(10, 160);
    effectText->setVisible(false);
    scene->addItem(effectText);

    // 主游戏循环定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::gameLoop);

    // 新增：问号生成定时器（每1秒检查一次）
    questionSpawnTimer = new QTimer(this);
    connect(questionSpawnTimer, &QTimer::timeout, this, &Game::onQuestionSpawnTimer);

    // 新增：效果持续时间定时器
    effectDurationTimer = new QTimer(this);
    connect(effectDurationTimer, &QTimer::timeout, this, [this]() { resetEffects(); });

    // 新增：物理数据更新定时器（更频繁地更新显示）
    physicsUpdateTimer = new QTimer(this);
    connect(physicsUpdateTimer, &QTimer::timeout, this, &Game::updatePhysicsDisplay);

    realTime.invalidate();
    setFixedSize(400, 600);
    scene->setSceneRect(0, 0, 400, 600);
    scene->setBackgroundBrush(QBrush(QImage(":/assets/images/background-day.png").scaled(400, 600)));

    // 取消滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    qDebug() << "Game初始化完成，当前角色:" << characterName << "是否为夕泡泡:" << isXiCharacter;

    invincibilityTimer = new QTimer(this);
    connect(invincibilityTimer, &QTimer::timeout, this, [this]() {
        if (realTime.elapsed() - invincibilityStartTime >= INVINCIBILITY_DURATION) {
            isInvincible = false;
            invincibilityTimer->stop();
            if (bird) {
                bird->setOpacity(1.0); // 恢复透明度
            }
            qDebug() << "无敌状态结束";
        }
    });
}

void Game::updatePhysicsDisplay() {
    if (!bird || !physicsText) return;

    // 获取bird的当前物理参数
    qreal currentGravity = bird->getGravity();
    qreal currentLift = bird->getLift();
    qreal currentVelocity = bird->getVelocity();

    // 格式化显示
    QString physicsInfo = QString("物理: 重力: %1 升力: %2 速度: %3")
                              .arg(currentGravity, 0, 'f', 2) // 重力显示2位小数
                              .arg(currentLift, 0, 'f', 1)    // 升力显示1位小数
                              .arg(currentVelocity, 0, 'f', 1); // 速度显示1位小数

    physicsText->setPlainText(physicsInfo);

    // 根据数值改变颜色（可选效果）
    if (currentGravity > 1.0) {
        physicsText->setDefaultTextColor(QColor(255, 100, 100)); // 红色表示高重力
    } else if (currentGravity < 0.9) {
        physicsText->setDefaultTextColor(QColor(100, 255, 100)); // 绿色表示低重力
    } else {
        physicsText->setDefaultTextColor(QColor(255, 200, 100)); // 橙色表示中等
    }
}

void Game::powerOn() {
    // 重置游戏状态
    isGameOver = false;
    isPaused = false;

    // 重置分数和速度
    score = 0;
    globalSpeedFactor = InitialSpeed;
    difficultyLevel = 0;

    // 重置时间
    realTime.start();
    lastSpeedRaiseMS = 0;
    lastQuestionSpawnTime = 0;

    // 重置无敌状态
    isInvincible = false;
    if (invincibilityTimer && invincibilityTimer->isActive()) {
        invincibilityTimer->stop();
    }
    if (bird) {
        bird->setOpacity(1.0);
    }

    // 更新显示
    if (scoreText) {
        scoreText->setPlainText(QString("得分: %1").arg(score));
    }
    if (speedText) {
        speedText->setPlainText(QString("Speed: x%1").arg(globalSpeedFactor, 0, 'f', 1));
    }

    // 清理之前的游戏对象（如果有）
    clearAllQuestionItems();
    for (Pipe* pipe : pipes) {
        scene->removeItem(pipe);
        delete pipe;
    }
    pipes.clear();

    // 如果是夕泡泡角色，需要特殊重置
    if (isXiCharacter && bird) {
        // 获取夕泡泡角色的基础配置
        CharacterSelection::CharacterInfo xiInfo = CharacterSelection::getCharacterInfo(3); // 夕泡泡是角色3

        // 重置鸟的大小为夕泡泡的基础大小
        bird->updateBirdSize(xiInfo.size);

        // 根据当前难度重置物理参数
        switch(pendingDifficulty) {  // 使用主菜单中设置的难度
        case 0: // 简单
            initialGravity = EASY_GRAVITY;
            initialLift = EASY_LIFT;
            break;
        case 1: // 中等
            initialGravity = MEDIUM_GRAVITY;
            initialLift = MEDIUM_LIFT;
            break;
        case 2: // 困难
            initialGravity = HARD_GRAVITY;
            initialLift = HARD_LIFT;
            break;
        default:
            initialGravity = MEDIUM_GRAVITY;
            initialLift = MEDIUM_LIFT;
        }

        // 更新bird的物理参数为当前难度的基础值
        bird->setDifficultyParameters(initialGravity, initialLift);
        bird->applyDifficultyParameters();

        // 重置效果状态
        resetEffects();

        qDebug() << "夕泡泡角色已重置: 大小=" << xiInfo.size
                 << "重力=" << initialGravity
                 << "升力=" << initialLift;
    }

    // 重置鸟的位置和状态
    if (bird) {
        bird->reset();
    }

    // 启动游戏循环
    if (!timer->isActive()) {
        timer->start(20);
    }

    // 启动物理更新定时器
    if (physicsUpdateTimer && !physicsUpdateTimer->isActive()) {
        physicsUpdateTimer->start(200);
    }

    // 如果是夕泡泡角色，启动问号生成定时器
    if (isXiCharacter && !questionSpawnTimer->isActive()) {
        questionSpawnTimer->start(1000);
        qDebug() << "启动问号生成定时器（夕泡泡）";
    }

    // 立即更新物理显示
    updatePhysicsDisplay();

    qDebug() << "游戏已启动（powerOn），角色:" << characterName
             << "难度:" << pendingDifficulty;
}

void Game::setDifficulty(int level) {
    pendingDifficulty = level;
    switch(level) {
    case 0: // 简单
        initialGravity = EASY_GRAVITY;
        initialLift = EASY_LIFT;
        pipeSpeed = EASY_SPEED;
        speedIncrease = EASY_SPEED_INCREASE;
        gravityIncreaseRate = EASY_GRAVITY_INCREASE_RATE;
        liftDecreaseRate = EASY_LIFT_DECREASE_RATE;
        currentPipeGap = EASY_GAP;
        difficultyName = "休闲模式";
        break;
    case 1: // 中等
        initialGravity = MEDIUM_GRAVITY;
        initialLift = MEDIUM_LIFT;
        pipeSpeed = MEDIUM_SPEED;
        speedIncrease = MEDIUM_SPEED_INCREASE;
        gravityIncreaseRate = MEDIUM_GRAVITY_INCREASE_RATE;
        liftDecreaseRate = MEDIUM_LIFT_DECREASE_RATE;
        currentPipeGap = MEDIUM_GAP;
        difficultyName = "普通模式";
        break;
    case 2: // 困难
        initialGravity = HARD_GRAVITY;
        initialLift = HARD_LIFT;
        pipeSpeed = HARD_SPEED;
        speedIncrease = HARD_SPEED_INCREASE;
        gravityIncreaseRate = HARD_GRAVITY_INCREASE_RATE;
        liftDecreaseRate = HARD_LIFT_DECREASE_RATE;
        currentPipeGap = HARD_GAP;
        difficultyName = "挑战模式";
        break;
    default:
        initialGravity = MEDIUM_GRAVITY;
        initialLift = MEDIUM_LIFT;
        pipeSpeed = MEDIUM_SPEED;
        speedIncrease = MEDIUM_SPEED_INCREASE;
        gravityIncreaseRate = MEDIUM_GRAVITY_INCREASE_RATE;
        liftDecreaseRate = MEDIUM_LIFT_DECREASE_RATE;
        currentPipeGap = MEDIUM_GAP;
        difficultyName = "普通模式";
    }

    qDebug() << "设置难度: " << difficultyName << " 重力: " << initialGravity << " 跳跃力: " << initialLift << " 管道间隙: " << currentPipeGap;

    // 更新难度显示文本
    if (difficultyText) {
        difficultyText->setPlainText(QString("模式: %1").arg(difficultyName));
    }

    // 创建或更新bird对象
    if (!bird) {
        // 从全局配置获取角色设置
        GlobalCharacterConfig config = getGlobalCharacterConfig();
        qDebug() << "创建bird使用角色配置: " << "名称: " << config.name << "图片: " << config.imagePath << "尺寸: " << config.size << "重力: " << config.gravity << "跳跃力: " << config.lift;

        // 使用全局配置创建bird，但使用当前难度的重力/跳跃力覆盖角色配置
        bird = new Bird(
            initialGravity, // 使用当前难度的重力
            initialLift,    // 使用当前难度的跳跃力
            config.size,    // 使用角色配置的尺寸
            config.imagePath // 使用角色配置的贴图
            );
        scene->addItem(bird);

        // 更新角色显示（确保与创建的一致）
        characterName = config.name;
        isXiCharacter = (characterName == "夕泡泡");
        if (characterText) {
            characterText->setPlainText(QString("角色: %1").arg(characterName));
        }

        // 启动游戏计时器
        if (!timer->isActive()) {
            timer->start(20);
        }

        // 确保物理更新定时器启动
        if (physicsUpdateTimer && !physicsUpdateTimer->isActive()) {
            physicsUpdateTimer->start(200); // 每200ms更新一次
            qDebug() << "物理更新定时器已启动，间隔200ms";
        }

        // 如果是夕泡泡角色，启动问号生成定时器
        if (isXiCharacter && !questionSpawnTimer->isActive()) {
            questionSpawnTimer->start(1000); // 每1秒检查一次
            lastQuestionSpawnTime = 0; // 重置时间
            qDebug() << "夕泡泡角色，启动问号生成定时器";
        }

        realTime.start(); // 世界时钟开始走动
        lastSpeedRaiseMS = 0; // 从 0 计时
    } else {
        // 更新已存在的bird对象的难度参数
        bird->setDifficultyParameters(initialGravity, initialLift);
        bird->applyDifficultyParameters();
        bird->reset();

        if (physicsUpdateTimer && !physicsUpdateTimer->isActive()) {
            physicsUpdateTimer->start(200);
        }

        // 如果是夕泡泡角色，确保问号定时器运行
        if (isXiCharacter && !questionSpawnTimer->isActive()) {
            questionSpawnTimer->start(1000);
            lastQuestionSpawnTime = realTime.elapsed();
        }

        qDebug() << "更新现有bird的难度参数";
    }

    updatePhysicsDisplay();
    qDebug() << "立即调用updatePhysicsDisplay()";
}

void Game::setCharacter(const CharacterSelection::CharacterInfo& characterInfo) {
    // 保存到全局配置
    GlobalCharacterConfig config;
    config.imagePath = characterInfo.imagePath;
    config.size = characterInfo.size;
    config.gravity = characterInfo.gravity; // 保存角色的基础参数
    config.lift = characterInfo.lift;       // 保存角色的基础参数
    config.name = characterInfo.name;
    setGlobalCharacterConfig(config);

    characterName = config.name;
    isXiCharacter = (characterName == "夕泡泡");

    qDebug() << "设置角色并保存到全局配置: " << config.toString();
    qDebug() << "是否为夕泡泡角色: " << isXiCharacter;

    // 更新显示
    if (characterText) {
        characterText->setPlainText(QString("角色: %1").arg(characterName));
    }

    // 如果是夕泡泡角色且游戏正在进行，启动问号生成定时器
    if (isXiCharacter && bird && timer->isActive()) {
        if (!questionSpawnTimer->isActive()) {
            questionSpawnTimer->start(1000); // 每1秒检查一次
            lastQuestionSpawnTime = realTime.elapsed();
            qDebug() << "启动问号生成定时器（夕泡泡专属）";
        }
    } else if (!isXiCharacter) {
        // 如果不是夕泡泡，停止问号生成并清除所有问号
        if (questionSpawnTimer->isActive()) {
            questionSpawnTimer->stop();
            qDebug() << "停止问号生成定时器（非夕泡泡角色）";
        }
        clearAllQuestionItems();
    }

    // 如果bird已经存在，立即更新（但保持当前难度）
    if (bird) {
        // 使用角色配置，但保持当前的难度重力/跳跃力
        Bird::CharacterSettings settings;
        settings.imagePath = characterInfo.imagePath;
        settings.size = characterInfo.size;
        settings.gravity = initialGravity; // 使用当前难度的重力，不是角色默认值
        settings.lift = initialLift;       // 使用当前难度的跳跃力，不是角色默认值
        bird->changeCharacter(settings);
        qDebug() << "立即更新现有bird的角色贴图和尺寸";
    }
}

void Game::setCharacterFromConfig(const GlobalCharacterConfig& config) {
    // 更新全局配置
    setGlobalCharacterConfig(config);
    characterName = config.name;
    isXiCharacter = (characterName == "夕泡泡");

    qDebug() << "从配置设置角色: " << config.toString();
    qDebug() << "是否为夕泡泡角色: " << isXiCharacter;

    // 更新显示
    if (characterText) {
        characterText->setPlainText(QString("角色: %1").arg(characterName));
    }

    // 如果切换到夕泡泡且游戏正在进行，启动问号生成定时器
    if (isXiCharacter && bird && timer->isActive()) {
        if (!questionSpawnTimer->isActive()) {
            questionSpawnTimer->start(1000); // 每1秒检查一次
            lastQuestionSpawnTime = realTime.elapsed();
            qDebug() << "启动问号生成定时器（夕泡泡专属）";
        }
    } else if (!isXiCharacter) {
        // 如果不是夕泡泡，停止问号生成并清除所有问号
        if (questionSpawnTimer->isActive()) {
            questionSpawnTimer->stop();
            qDebug() << "停止问号生成定时器（非夕泡泡角色）";
        }
        clearAllQuestionItems();
    }

    // 如果bird已经存在，立即更新
    if (bird) {
        Bird::CharacterSettings settings;
        settings.imagePath = config.imagePath;
        settings.size = config.size;
        settings.gravity = initialGravity;
        settings.lift = initialLift;
        bird->changeCharacter(settings);
    }
}

void Game::onQuestionSpawnTimer() {
    // 检查是否满足生成条件
    if (!isXiCharacter || !bird || pipes.isEmpty() || isPaused || isGameOver) {
        return;
    }

    qint64 currentTime = realTime.elapsed();
    // 如果是第一次生成，设置起始时间
    if (lastQuestionSpawnTime == 0) {
        lastQuestionSpawnTime = currentTime;
        return;
    }

    qint64 timeDiff = currentTime - lastQuestionSpawnTime;
    if (timeDiff >= QuestionCreateTime) { // 10秒
        qDebug() << "已过10秒（" << timeDiff << "ms），尝试生成问号...";
        spawnQuestionItem();
        lastQuestionSpawnTime = currentTime;
    }
}

void Game::spawnQuestionItem() {
    // 找到最新的管道
    if (pipes.isEmpty()) {
        qDebug() << "无法生成问号：管道列表为空";
        return;
    }

    Pipe* latestPipe = pipes.last();
    if (!latestPipe) {
        qDebug() << "无法生成问号：最新管道指针为空";
        return;
    }

    if (latestPipe->hasQuestionItem()) {
        qDebug() << "无法生成问号：该管道已经有问号了";
        return; // 该管道已经有问号了
    }

    // 检查管道是否在合适的范围内
    if (latestPipe->x() < 100 || latestPipe->x() > 350) {
        qDebug() << "无法生成问号：管道位置不合适 x=" << latestPipe->x();
        return;
    }

    // 创建问号道具
    QuestionItem* question = new QuestionItem(0, 0);

    // 将问号附加到管道上
    latestPipe->attachQuestionItem(question);
    questionItems.append(question);
    qDebug() << "成功生成问号道具！";
}

void Game::applyQuestionEffect(QuestionItem::EffectType effect) {
    qDebug() << "开始应用问号效果:" << effect;

      activateInvincibility();

    if (!isXiCharacter || !bird) {
        qDebug() << "应用效果失败: 不是夕泡泡或bird不存在";
        return;
    }

    QString effectMsg;
    switch (effect) {
    case QuestionItem::SpeedDecrease: // 速度减小
        globalSpeedFactor = qMax(InitialSpeed, globalSpeedFactor - 0.2);
        for (Pipe* pipe : pipes) {
            if (pipe) pipe->setGlobalSpeedFactor(globalSpeedFactor);
        }
        speedText->setPlainText(QString("Speed: x%1").arg(globalSpeedFactor, 0, 'f', 1));
        effectMsg = "效果: 速度减小!";
        break;
    case QuestionItem::GapIncrease: // 管道间隙增加（仅对后续管道生效）
        currentPipeGap += 20;
        effectMsg = "效果: 管道间隙增加!";
        break;
    case QuestionItem::GravityDecrease: // 重力减小
        if (bird) {
            bird->setDifficultyParameters(qMax(0.5, bird->getGravity() * 0.8), bird->getLift());
            bird->applyDifficultyParameters();
        }
        effectMsg = "效果: 重力减小!";
        break;
    case QuestionItem::LiftIncrease: // 升力增加（升力是负值，增加意味着更负）
        if (bird) {
            bird->setDifficultyParameters(bird->getGravity(), bird->getLift() * 0.9);
            bird->applyDifficultyParameters();
        }
        effectMsg = "效果: 升力增加!";
        break;
    case QuestionItem::BirdShrink: // 鸟变小
        if (bird) {
            bird->updateBirdSize(qMax(30, bird->getBirdSize() - 20));
        }
        effectMsg = "效果: 鸟变小!";
        break;
    case QuestionItem::BirdEnlarge: // 鸟变大
        if (bird) {
            bird->updateBirdSize(qMin(100, bird->getBirdSize() + 20));
        }
        effectMsg = "效果: 鸟变大!";
        break;
    }

    // 显示效果消息
    showEffectMessage(effectMsg);

    // 设置效果持续时间（5秒）
    if (effectDurationTimer->isActive()) {
        effectDurationTimer->stop();
    }
    effectDurationTimer->start(5000);
    isEffectActive = true;
    currentEffect = effectMsg;
    qDebug() << "成功应用问号效果:" << effectMsg;
}

void Game::activateInvincibility() {
    isInvincible = true;
    invincibilityStartTime = realTime.elapsed();

    if (!invincibilityTimer->isActive()) {
        invincibilityTimer->start(100); // 每100ms检查一次
    }

    // 视觉提示：让小鸟闪烁或半透明
    if (bird) {
        bird->setOpacity(0.7); // 半透明效果
    }

    qDebug() << "激活无敌状态，持续时间:" << INVINCIBILITY_DURATION << "ms";
}

void Game::showEffectMessage(const QString& message) {
    if (effectText) {
        effectText->setPlainText(message);
        effectText->setVisible(true);
        // 3秒后自动隐藏
        QTimer::singleShot(3000, this, [this]() {
            if (effectText) {
                effectText->setVisible(false);
            }
        });
    }
}

void Game::resetEffects() {
    // 重置效果状态
    isEffectActive = false;
    currentEffect = "";
    if (effectText) {
        effectText->setVisible(false);
    }
    if (effectDurationTimer->isActive()) {
        effectDurationTimer->stop();
    }
    qDebug() << "问号效果已结束";
}

void Game::clearAllQuestionItems() {
    for (QuestionItem* question : questionItems) {
        if (question) {
            scene->removeItem(question);
            delete question;
        }
    }
    questionItems.clear();

    // 也从管道中移除问号（仅清除指针，不 delete）
    for (Pipe* pipe : pipes) {
        pipe->detachQuestionItem();
    }
    qDebug() << "已清除所有问号道具";
}

void Game::keyPressEvent(QKeyEvent* event) {
    // 空格键：死亡重启 / 正常跳跃（暂停时屏蔽）
    if (event->key() == Qt::Key_Space) {
        if (isGameOver) {
            if (waiting) return;
            restartGame();
        } else if (!isPaused && bird) {
            bird->flap();
        }
        event->accept();
        return;
    }

    // ----------- ESC = 暂停/继续 切换 -----------
    if (event->key() == Qt::Key_Escape) {
        if (isGameOver) return; // 游戏已结束就别暂停
        isPaused = !isPaused;
        if (isPaused) {
            // 进入暂停
            timer->stop(); // 停渲染
            realTime.invalidate(); // 停世界时钟
            if (questionSpawnTimer->isActive()) {
                questionSpawnTimer->stop(); // 暂停问号生成
            }
            // 黑色半透明遮罩
            pauseOverlay = new QGraphicsRectItem(scene->sceneRect());
            pauseOverlay->setBrush(QBrush(QColor(0, 0, 0, 127)));
            pauseOverlay->setZValue(1000);
            scene->addItem(pauseOverlay);
            // 暂停提示图
            QPixmap pix(":/assets/images/escloop.png");
            if (!pix.isNull()) {
                pausePix = new QGraphicsPixmapItem(pix);
                pausePix->setZValue(1001);
                pausePix->setPos((width() - pix.width()) / 2, (height() - pix.height()) / 2);
                scene->addItem(pausePix);
            }
        } else {
            // 解除暂停
            timer->start(20); // 恢复渲染
            realTime.start(); // 恢复世界时钟
            // 如果是夕泡泡，恢复问号生成定时器
            if (isXiCharacter && !questionSpawnTimer->isActive()) {
                questionSpawnTimer->start(1000);
            }
            if (pauseOverlay) {
                scene->removeItem(pauseOverlay);
                delete pauseOverlay;
                pauseOverlay = nullptr;
            }
            if (pausePix) {
                scene->removeItem(pausePix);
                delete pausePix;
                pausePix = nullptr;
            }
        }
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        shutdownGame(); // ← 只留这一行就够了
        emit closed();
        hide();
        event->accept();
        return;
    }

    // 其他键忽略
    event->ignore();
}

void Game::increaseGameDifficulty() {
    if (!bird) return;
    difficultyLevel++;
    globalSpeedFactor += speedIncrease; //根据难度改变，默认为中等
    if (globalSpeedFactor > MaxSpeed) globalSpeedFactor = MaxSpeed;
    for (Pipe* pipe : pipes) {
        pipe->setGlobalSpeedFactor(globalSpeedFactor);
    }
    bird->increaseDifficulty(gravityIncreaseRate, liftDecreaseRate);
    speedText->setPlainText(QString("Speed: x%1").arg(globalSpeedFactor, 0, 'f', 1));
    if (difficultyLevel % 3 == 0) {
        for (Pipe* pipe : pipes) {
            pipe->setSpeed(pipe->getSpeed() * 1.05);
        }
    }
}

void Game::restartGame() {
    timer->stop();
    if (questionSpawnTimer->isActive()) {
        questionSpawnTimer->stop();
    }
    if (physicsUpdateTimer) {
        physicsUpdateTimer->stop();
    }

    // 清理所有问号
    clearAllQuestionItems();

    // 重置效果
    resetEffects();

    isInvincible = false;
    if (invincibilityTimer && invincibilityTimer->isActive()) {
        invincibilityTimer->stop();
    }
    if (bird) {
        bird->setOpacity(1.0);
    }

    for (Pipe* pipe : std::as_const(pipes)) {
        scene->removeItem(pipe);
        delete pipe;
    }
    pipes.clear();

    if (bird) {
        bird->reset();
    }

    score = 0;
    globalSpeedFactor = InitialSpeed;
    difficultyLevel = 0;
    realTime.start(); // 重新从 0 开始
    lastSpeedRaiseMS = 0;
    lastQuestionSpawnTime = 0;

    scoreText->setPlainText(QString("得分: %1").arg(score));
    speedText->setPlainText(QString("Speed: x%1").arg(globalSpeedFactor, 0, 'f', 1));
    difficultyText->setPlainText(QString("模式: %1").arg(difficultyName));
    characterText->setPlainText(QString("角色: %1").arg(characterName));

    // 清理游戏结束画面
    QList<QGraphicsItem*> items = scene->items();
    QList<QGraphicsItem*> itemsToRemove;
    for (QGraphicsItem* item : items) {
        if (QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            if (pixmapItem->pixmap().cacheKey() == QPixmap(":/assets/images/gameover.png").cacheKey()) {
                itemsToRemove.append(item);
            }
        } else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item)) {
            if (textItem->toPlainText() == "按空格键重新开始") {
                itemsToRemove.append(textItem);
            }
        }
    }
    for (QGraphicsItem* item : itemsToRemove) {
        scene->removeItem(item);
        delete item;
    }

    isGameOver = false;
    isPaused = false;
    timer->start(20);

    // 如果是夕泡泡角色，启动问号生成定时器
    if (isXiCharacter) {
        questionSpawnTimer->start(1000);
        qDebug() << "游戏重新开始，启动问号生成定时器（夕泡泡）";
    }

    physicsUpdateTimer->start(200);
    qDebug() << "游戏重新开始，使用角色: " << characterName;
    recordText->setPlainText(QString("最高得分: %1").arg(record));
}

void Game::gameLoop() {
    if (!bird || isPaused) return;

    // ---- 真实时间补帧：暂停期间自动停走 ----
    qint64 now = realTime.elapsed();
    qint64 delta = now - lastSpeedRaiseMS;
    int ticks = int(delta / AddSpeedPerTime); // 应涨几次
    if (ticks > 0) {
        for (int i = 0; i < ticks; ++i) increaseGameDifficulty(); // 内部已有 MaxSpeed 保护
        lastSpeedRaiseMS += ticks * AddSpeedPerTime;
    }

    bird->updatePosition();

    // ==== 检查与问号的碰撞 ====
    // 使用临时列表避免在迭代时修改原列表
    QList<QuestionItem*> collectedQuestions;
    for (QuestionItem* question : questionItems) {
        if (!question) {
            qDebug() << "警告：发现空问号指针，跳过";
            continue; // 跳过空指针
        }
        if (question->isCollected()) {
            // 如果已经收集但还在列表中，标记为待删除
            collectedQuestions.append(question);
            continue;
        }
        // 检查碰撞
        if (bird && bird->collidesWithItem(question)) {
            qDebug() << "夕泡泡吃到问号! 效果类型:" << question->getEffectDescription();
            // ✅ 关键修复：直接调用而不是通过信号
            applyQuestionEffect(question->getEffectType());
            // 标记为已收集
            question->setCollected(true);
            // 添加到待处理列表
            collectedQuestions.append(question);
        }
    }

    // 处理已收集的问号
    for (QuestionItem* question : collectedQuestions) {
        // 从主列表中移除
        if (questionItems.contains(question)) {
            questionItems.removeOne(question);
        }
        // 从场景中移除
        if (question && scene->items().contains(question)) {
            scene->removeItem(question);
        }
        // 从管道中解绑（仅清除指针）
        for (Pipe* pipe : pipes) {
            if (pipe && pipe->hasQuestionItem() && pipe->getQuestionItem() == question) {
                pipe->detachQuestionItem();
                break;
            }
        }
        // 删除对象
        if (question) {
            delete question;
        }
    }
    // 清理临时列表
    collectedQuestions.clear();

    // ==== 管道生成 ====
    if (pipes.isEmpty() || pipes.last()->x() < 200) {
        // 传递当前难度级别到Pipe构造函数
        Pipe* pipe = new Pipe(pipeSpeed, difficultyLevel);
        pipe->setGlobalSpeedFactor(globalSpeedFactor);
        pipes.append(pipe);
        scene->addItem(pipe);
        qDebug() << "生成新管道，位置: x=" << pipe->x() << "，当前管道数量:" << pipes.size();
    }

    // ==== 移动管道并检查碰撞 ====
    auto it = pipes.begin();
    while (it != pipes.end()) {
        Pipe* pipe = *it;
        if (!pipe) {
            // 空指针，直接移除
            it = pipes.erase(it);
            continue;
        }

        // 移动管道
        pipe->movePipe();

        // 检查与鸟的碰撞
        if (bird && bird->collidesWithItem(pipe)) {
            if (isInvincible) {
                ++it;  // 继续处理下一个管道
                continue;
            }
            qDebug() << "撞到管道！游戏结束";
            // 停止所有计时器
            timer->stop();
            if (questionSpawnTimer && questionSpawnTimer->isActive()) {
                questionSpawnTimer->stop();
            }
            if (physicsUpdateTimer && physicsUpdateTimer->isActive()) {
                physicsUpdateTimer->stop();
            }
            if (effectDurationTimer && effectDurationTimer->isActive()) {
                effectDurationTimer->stop();
            }

            // 显示游戏结束画面
            QGraphicsPixmapItem* gameOverItem = scene->addPixmap(QPixmap(":/assets/images/gameover.png"));
            gameOverItem->setPos(this->width() / 2 - gameOverItem->pixmap().width() / 2,
                                 this->height() / 2 - gameOverItem->pixmap().height() / 2);
            gameOverItem->setZValue(1000);
            isGameOver = true;
            waiting = true; // 开始冷却

            // 0.5秒后允许重新开始
            QTimer::singleShot(500, this, [this]() {
                waiting = false;
                qDebug() << "冷却结束，可以重新开始游戏";
            });

            // 刷新历史最高分
            if (score > record) {
                record = score;
                saveRecord();
                recordText->setPlainText(QString("最高得分: %1").arg(record));
                qDebug() << "新纪录:" << record;
            }

            // 显示重新开始提示
            QGraphicsTextItem* restartText = new QGraphicsTextItem("按空格键重新开始");
            restartText->setDefaultTextColor(Qt::black);
            restartText->setFont(QFont("Arial", 12, QFont::Bold));
            restartText->setZValue(1001);
            restartText->setPos(this->width() / 2 - restartText->boundingRect().width() / 2,
                                this->height() / 2 + gameOverItem->pixmap().height() / 2 + 10);
            scene->addItem(restartText);

            // 清理问号（避免内存泄漏）
            clearAllQuestionItems();
            return;
        }

        // 计分逻辑：当鸟通过管道时
        if (pipe->x() + pipe->boundingRect().width() < bird->x() && !pipe->isPassed) {
            score++;
            pipe->isPassed = true;
            scoreText->setPlainText(QString("得分: %1").arg(score));
            qDebug() << "得分:" << score;
        }

        // 清理超出屏幕的管道
        if (pipe->x() < -60) {
            // 先移除管道关联的问号（如果有）
            if (pipe->hasQuestionItem()) {
                QuestionItem* question = pipe->getQuestionItem();
                if (question && questionItems.contains(question)) {
                    questionItems.removeOne(question);
                }
                if (question && scene->items().contains(question)) {
                    scene->removeItem(question);
                }
                // ✅ 关键：只 detach（不清除内存），因为 Game 已在上面统一 delete
                pipe->detachQuestionItem();
                // 注意：这里不再 delete question，因为在 collectedQuestions 或 clearAllQuestionItems 中已处理
            }

            // 移除管道本身
            scene->removeItem(pipe);
            delete pipe;
            it = pipes.erase(it);
        } else {
            ++it;
        }
    }

    // ==== 清理超出屏幕的问号（兜底）====
    auto qIt = questionItems.begin();
    while (qIt != questionItems.end()) {
        QuestionItem* question = *qIt;
        if (!question) {
            // 空指针，直接移除
            qIt = questionItems.erase(qIt);
            continue;
        }
        // 检查问号是否超出屏幕或已收集
        if (question->x() < -50 || question->isCollected()) {
            // 从场景中移除
            if (scene->items().contains(question)) {
                scene->removeItem(question);
            }
            // 从管道解绑
            for (Pipe* pipe : pipes) {
                if (pipe && pipe->hasQuestionItem() && pipe->getQuestionItem() == question) {
                    pipe->detachQuestionItem();
                    break;
                }
            }
            // 删除对象
            delete question;
            qIt = questionItems.erase(qIt);
        } else {
            ++qIt;
        }
    }

    // ==== 检查鸟是否掉出屏幕底部或飞出顶部 ====
    if (bird->y() > this->height() || bird->y() < -bird->boundingRect().height()) {
        qDebug() << "鸟飞出屏幕边界！游戏结束";
        timer->stop();
        if (questionSpawnTimer && questionSpawnTimer->isActive()) {
            questionSpawnTimer->stop();
        }
        if (physicsUpdateTimer && physicsUpdateTimer->isActive()) {
            physicsUpdateTimer->stop();
        }
        if (effectDurationTimer && effectDurationTimer->isActive()) {
            effectDurationTimer->stop();
        }

        QGraphicsPixmapItem* gameOverItem = scene->addPixmap(QPixmap(":/assets/images/gameover.png"));
        gameOverItem->setPos(this->width() / 2 - gameOverItem->pixmap().width() / 2,
                             this->height() / 2 - gameOverItem->pixmap().height() / 2);
        gameOverItem->setZValue(1000);
        isGameOver = true;
        waiting = true;

        QTimer::singleShot(500, this, [this]() {
            waiting = false;
            qDebug() << "边界死亡冷却结束";
        });

        if (score > record) {
            record = score;
            saveRecord();
            recordText->setPlainText(QString("最高得分: %1").arg(record));
        }

        QGraphicsTextItem* restartText = new QGraphicsTextItem("按空格键重新开始");
        restartText->setDefaultTextColor(Qt::black);
        restartText->setFont(QFont("Arial", 12, QFont::Bold));
        restartText->setZValue(1001);
        restartText->setPos(this->width() / 2 - restartText->boundingRect().width() / 2,
                            this->height() / 2 + gameOverItem->pixmap().height() / 2 + 10);
        scene->addItem(restartText);

        clearAllQuestionItems();
    }

    // 更新物理数据显示（每帧更新）
    updatePhysicsDisplay();
}

void Game::shutdownGame() {
    qDebug() << "正在关闭游戏...";
    timer->stop();
    if (questionSpawnTimer->isActive()) {
        questionSpawnTimer->stop();
    }
    if (physicsUpdateTimer->isActive()) {
        physicsUpdateTimer->stop();
    }
    if (effectDurationTimer->isActive()) {
        effectDurationTimer->stop();
    }

    clearAllQuestionItems();

    for (Pipe* pipe : std::as_const(pipes)) {
        scene->removeItem(pipe);
        delete pipe;
    }
    pipes.clear();

    if (bird) {
        scene->removeItem(bird);
        delete bird;
        bird = nullptr;
    }

    // 移除所有UI文本项
    if (scoreText) { scene->removeItem(scoreText); delete scoreText; scoreText = nullptr; }
    if (recordText) { scene->removeItem(recordText); delete recordText; recordText = nullptr; }
    if (speedText) { scene->removeItem(speedText); delete speedText; speedText = nullptr; }
    if (difficultyText) { scene->removeItem(difficultyText); delete difficultyText; difficultyText = nullptr; }
    if (characterText) { scene->removeItem(characterText); delete characterText; characterText = nullptr; }
    if (physicsText) { scene->removeItem(physicsText); delete physicsText; physicsText = nullptr; }
    if (effectText) { scene->removeItem(effectText); delete effectText; effectText = nullptr; }

    if (pauseOverlay) { scene->removeItem(pauseOverlay); delete pauseOverlay; pauseOverlay = nullptr; }
    if (pausePix) { scene->removeItem(pausePix); delete pausePix; pausePix = nullptr; }

    // 清理场景中剩余的 game over 元素
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (dynamic_cast<QGraphicsPixmapItem*>(item) ||
            dynamic_cast<QGraphicsTextItem*>(item)) {
            scene->removeItem(item);
            delete item;
        }
    }

    qDebug() << "游戏资源已全部释放";
}

void Game::saveRecord() {
    QSettings settings("HUST", "FlappyBird");
    settings.setValue("record", record);
    qDebug() << "保存最高分:" << record;
}

void Game::loadRecord() {
    QSettings settings("HUST", "FlappyBird");
    record = settings.value("record", 0).toInt();
    qDebug() << "加载最高分:" << record;
}
