#include "mainmenu.h"
#include <QFont>
#include <QApplication>
#include <game.h>
MainMenu::MainMenu(Game* gameWidget, QWidget *parent) : QWidget(parent), game(gameWidget) {
    setupUI();
    applyStyle();
    setFixedSize(400, 600);  //(455,600)
    setWindowTitle("HUST小鸟 - 主菜单");
}

void MainMenu::setupUI() {
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // 标题
    titleLabel = new QLabel("HUST小鸟");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 按钮布局
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(20);

    // 创建按钮
    startButton = new QPushButton("开始游戏");
    difficultyButton = new QPushButton("难度选择");
    characterButton = new QPushButton("角色选择");
    exitButton = new QPushButton("退出游戏");

    // 连接信号
    connect(startButton, &QPushButton::clicked, this, &MainMenu::onStartGameClicked);
    connect(difficultyButton, &QPushButton::clicked, this, &MainMenu::onDifficultySelectClicked);
    connect(characterButton, &QPushButton::clicked, this, &MainMenu::onCharacterSelectClicked);
    connect(exitButton, &QPushButton::clicked, qApp, &QApplication::quit);

    // 添加到布局
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(difficultyButton);
    buttonLayout->addWidget(characterButton);
    buttonLayout->addWidget(exitButton);

    mainLayout->addLayout(buttonLayout);
}

void MainMenu::applyStyle() {
    // 设置窗口背景
    QPixmap backgroundPixmap(":/assets/images/3.png");
    if (!backgroundPixmap.isNull()) {
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(backgroundPixmap));
        this->setPalette(palette);
        this->setAutoFillBackground(true);
    } else {
        // 使用现有背景图或默认颜色
        backgroundPixmap = QPixmap(":/assets/images/background-day.png");
        if (!backgroundPixmap.isNull()) {
            backgroundPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
            QPalette palette;
            palette.setBrush(QPalette::Window, QBrush(backgroundPixmap));
            this->setPalette(palette);
            this->setAutoFillBackground(true);
        } else {
            setStyleSheet("background-color: #3498db;");
        }
    }

    // 标题样式
    QFont titleFont("Arial", 36, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: purple;");

    // 按钮样式
    QFont buttonFont("Arial", 16, QFont::Bold);

    startButton->setFont(buttonFont);
    difficultyButton->setFont(buttonFont);
    characterButton->setFont(buttonFont);
    exitButton->setFont(buttonFont);

    // 统一按钮样式
    QString buttonStyle =
        "QPushButton {"
        "   background-color: rgba(255, 255, 255, 200);"
        "   border: 2px solid #2c3e50;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "   width: 150px;"
        "   height: 50px;"
        "   color: #2c3e50;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 230);"
        "   border: 2px solid #34495e;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(236, 240, 241, 200);"
        "}";

    startButton->setStyleSheet(buttonStyle);
    difficultyButton->setStyleSheet(buttonStyle);
    characterButton->setStyleSheet(buttonStyle);
    exitButton->setStyleSheet(buttonStyle);
}

void MainMenu::onStartGameClicked() {
    game->powerOn();
    emit startGame(pendingDifficulty);   // 带难度发出
    hide();
}

void MainMenu::onDifficultySelectClicked() {
    emit showDifficultySelect();
    hide();
}

void MainMenu::onCharacterSelectClicked() {
    emit showCharacterSelect();
    hide();
}


