#include "difficultyselection.h"
#include <QFont>
#include <QStyle>
#include <QApplication>

DifficultySelection::DifficultySelection(QWidget *parent) : QWidget(parent) {
    setupUI();
    applyStyle();
    setFixedSize(400, 600);
    setWindowTitle("选择难度 - HUST小鸟");
}

void DifficultySelection::setupUI() {
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // 标题
    titleLabel = new QLabel("选择游戏模式");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 按钮布局
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(20);

    // 难度按钮
    easyButton = new QPushButton("休闲模式");
    mediumButton = new QPushButton("普通模式");
    hardButton = new QPushButton("挑战模式");

    // 连接信号
    connect(easyButton, &QPushButton::clicked, this, &DifficultySelection::onEasyClicked);
    connect(mediumButton, &QPushButton::clicked, this, &DifficultySelection::onMediumClicked);
    connect(hardButton, &QPushButton::clicked, this, &DifficultySelection::onHardClicked);

    // 添加到布局
    buttonLayout->addWidget(easyButton);
    buttonLayout->addWidget(mediumButton);
    buttonLayout->addWidget(hardButton);

    mainLayout->addLayout(buttonLayout);
}

void DifficultySelection::applyStyle() {
    // 设置窗口背景图片
    QPixmap backgroundPixmap(":/assets/images/3.png");  // 或者使用其他图片
    if (!backgroundPixmap.isNull()) {
        // 缩放背景图片
        //backgroundPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding);

        // 创建调色板设置背景
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(backgroundPixmap));
        this->setPalette(palette);
        this->setAutoFillBackground(true);
    } else {
        // 如果图片加载失败，使用默认颜色
        setStyleSheet("background-color: #ecf0f1;");
    }

    // 标题样式
    QFont titleFont("Arial", 36, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: purple;");

    // 按钮字体
    QFont buttonFont("Arial", 16, QFont::Bold);
    easyButton->setFont(buttonFont);
    mediumButton->setFont(buttonFont);
    hardButton->setFont(buttonFont);

    // 简单模式按钮
    easyButton->setStyleSheet(
        "background-color: #27ae60;"
        "color: white;"  // 直接设置颜色
        "border: 2px solid #2ecc71;"
        "border-radius: 8px;"
        "padding: 10px;"
        "min-width: 150px;"
        "min-height: 40px;"
        );

    // 中等模式按钮
    mediumButton->setStyleSheet(
        "background-color: #f39c12;"
        "color: white;"
        "border: 2px solid #f1c40f;"
        "border-radius: 8px;"
        "padding: 10px;"
        "min-width: 150px;"
        "min-height: 40px;"
        );

    // 困难模式按钮
    hardButton->setStyleSheet(
        "background-color: #e74c3c;"
        "color: white;"
        "border: 2px solid #c0392b;"
        "border-radius: 8px;"
        "padding: 10px;"
        "min-width: 150px;"
        "min-height: 40px;"
        );
}

void DifficultySelection::onEasyClicked() {
    emit difficultySelected(0);
    hide();
}

void DifficultySelection::onMediumClicked() {
    emit difficultySelected(1);
    hide();
}

void DifficultySelection::onHardClicked() {
    emit difficultySelected(2);
    hide();
}
