#include "characterselection.h"
#include <QFont>
#include <QDebug>
#include <QPainter>
#include <QGridLayout>
#include <QSpacerItem>
#include <QtCore>

// 初始化角色信息
QMap<int, CharacterSelection::CharacterInfo> CharacterSelection::characterMap = {
    {1, {"经典小鸟", ":/assets/images/b1.png", 50, 0.98, -10.0,
         "平衡型角色\n适合新手玩家"}},
    {2, {"黍泡泡", ":/assets/images/b2.png", 100, 0.85, -12.0,
         "傻乎乎的"}},
    {3, {"夕泡泡", ":/assets/images/b3.png", 90, 1.15, -8.0,
         "凶巴巴的<br>技能：随机生成问号，失去后获得4秒无敌和一个随机效果"}}
};

CharacterSelection::CharacterSelection(QWidget *parent) : QWidget(parent), currentSelectedId(1) {
    setupUI();
    applyStyle();
    setFixedSize(500, 600);
    setWindowTitle("角色选择 - HUST小鸟");

    // 默认预览第一个角色
    updateCharacterPreview(1);
}

void CharacterSelection::setupUI() {
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    titleLabel = new QLabel("选择你的角色");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 创建中央区域布局
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(30);

    // 左侧：角色预览区域
    QGroupBox *previewGroup = new QGroupBox("角色预览");
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);

    // 角色图像预览
    characterPreview = new QLabel();
    characterPreview->setAlignment(Qt::AlignCenter);
    characterPreview->setMinimumSize(200, 200);
    characterPreview->setStyleSheet("background-color: rgba(255, 255, 255, 100); border-radius: 10px;");

    // 角色属性显示
    characterStats = new QLabel();
    characterStats->setAlignment(Qt::AlignCenter);
    characterStats->setWordWrap(true);
    characterStats->setStyleSheet("background-color: rgba(255, 255, 255, 150); border-radius: 5px; padding: 10px;");

    previewLayout->addWidget(characterPreview);
    previewLayout->addWidget(characterStats);

    // 右侧：角色选择按钮区域
    QGroupBox *selectionGroup = new QGroupBox("可选角色");
    QVBoxLayout *selectionLayout = new QVBoxLayout(selectionGroup);
    selectionLayout->setSpacing(15);

    // 创建角色按钮
    character1Button = new QPushButton("经典小鸟");
    character2Button = new QPushButton("黍泡泡");
    character3Button = new QPushButton("夕泡泡");

    // 连接信号
    connect(character1Button, &QPushButton::clicked, this, &CharacterSelection::onCharacter1Clicked);
    connect(character2Button, &QPushButton::clicked, this, &CharacterSelection::onCharacter2Clicked);
    connect(character3Button, &QPushButton::clicked, this, &CharacterSelection::onCharacter3Clicked);

    // 添加到布局
    selectionLayout->addWidget(character1Button);
    selectionLayout->addWidget(character2Button);
    selectionLayout->addWidget(character3Button);
    selectionLayout->addStretch();

    centerLayout->addWidget(previewGroup);
    centerLayout->addWidget(selectionGroup);

    mainLayout->addLayout(centerLayout);

    // 底部按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

    backButton = new QPushButton("返回主菜单");
    confirmButton = new QPushButton("确定选择");

    // 连接信号
    connect(backButton, &QPushButton::clicked, this, &CharacterSelection::onBackClicked);
    connect(confirmButton, &QPushButton::clicked, this, [this]() {
        if (currentSelectedId > 0) {
            emit characterSelected(getCharacterInfo(currentSelectedId));
        }
    });

    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(confirmButton);

    mainLayout->addLayout(buttonLayout);
}

void CharacterSelection::applyStyle() {
    // 设置窗口背景
    QPixmap backgroundPixmap(":/assets/images/background-day.png");
    if (!backgroundPixmap.isNull()) {
        backgroundPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(backgroundPixmap));
        this->setPalette(palette);
        this->setAutoFillBackground(true);
    } else {
        setStyleSheet("background-color: #ecf0f1;");
    }

    // 标题样式
    QFont titleFont("Arial", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50; background-color: rgba(255, 255, 255, 150); padding: 10px; border-radius: 10px;");

    // 分组框样式
    QString groupBoxStyle =
        "QGroupBox {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   color: #34495e;"
        "   background-color: rgba(255, 255, 255, 120);"
        "   border: 2px solid #3498db;"
        "   border-radius: 10px;"
        "   padding-top: 15px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 10px;"
        "}";

    // 按钮样式
    QFont buttonFont("Arial", 14);

    // 角色按钮样式
    QString characterButtonStyle =
        "QPushButton {"
        "   background-color: rgba(52, 152, 219, 180);"
        "   border: 2px solid #2980b9;"
        "   border-radius: 8px;"
        "   padding: 15px;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   text-align: left;"
        "   padding-left: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(41, 128, 185, 180);"
        "   border: 2px solid #1c5d87;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(52, 152, 219, 150);"
        "}";

    character1Button->setStyleSheet(characterButtonStyle);
    character2Button->setStyleSheet(characterButtonStyle);
    character3Button->setStyleSheet(characterButtonStyle);

    character1Button->setFont(buttonFont);
    character2Button->setFont(buttonFont);
    character3Button->setFont(buttonFont);

    // 底部按钮样式
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(231, 76, 60, 180);"
        "   border: 2px solid #c0392b;"
        "   border-radius: 6px;"
        "   padding: 12px 25px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(192, 57, 43, 180);"
        "}");

    confirmButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(46, 204, 113, 180);"
        "   border: 2px solid #27ae60;"
        "   border-radius: 6px;"
        "   padding: 12px 25px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(39, 174, 96, 180);"
        "}");

    backButton->setFont(buttonFont);
    confirmButton->setFont(buttonFont);
}

void CharacterSelection::updateCharacterPreview(int characterId) {
    if (!characterMap.contains(characterId)) return;

    currentSelectedId = characterId;
    const CharacterInfo& info = characterMap[characterId];

    // 更新预览图像
    QPixmap pixmap(info.imagePath);
    if (!pixmap.isNull()) {
        // 缩放图片以适应预览区域
        pixmap = pixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        characterPreview->setPixmap(pixmap);
    }

    // 更新属性显示
    QString statsText = QString(
                            "<b>%1</b><br><br>"
                            "尺寸: %2px<br>"
                            "重力: %3<br>"
                            "跳跃力: %4<br><br>"
                            "%5"
                            ).arg(info.name)
                            .arg(info.size)
                            .arg(info.gravity, 0, 'f', 2)
                            .arg(info.lift, 0, 'f', 1)
                            .arg(info.description);

    characterStats->setText(statsText);

    // 更新按钮选中状态
    QString selectedStyle =
        "QPushButton {"
        "   background-color: rgba(241, 196, 15, 200);"
        "   border: 3px solid #f39c12;"
        "   border-radius: 8px;"
        "   padding: 15px;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   text-align: left;"
        "   padding-left: 20px;"
        "}";

    QString normalStyle =
        "QPushButton {"
        "   background-color: rgba(52, 152, 219, 180);"
        "   border: 2px solid #2980b9;"
        "   border-radius: 8px;"
        "   padding: 15px;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   text-align: left;"
        "   padding-left: 20px;"
        "}";

    character1Button->setStyleSheet(characterId == 1 ? selectedStyle : normalStyle);
    character2Button->setStyleSheet(characterId == 2 ? selectedStyle : normalStyle);
    character3Button->setStyleSheet(characterId == 3 ? selectedStyle : normalStyle);
}

CharacterSelection::CharacterInfo CharacterSelection::getCharacterInfo(int characterId) {
    if (characterMap.contains(characterId)) {
        return characterMap[characterId];
    }
    // 返回默认角色（经典小鸟）
    return characterMap[1];
}

void CharacterSelection::onBackClicked() {
    emit backToMainMenu();
    hide();
}

void CharacterSelection::onCharacter1Clicked() {
    updateCharacterPreview(1);
}

void CharacterSelection::onCharacter2Clicked() {
    updateCharacterPreview(2);
}

void CharacterSelection::onCharacter3Clicked() {
    updateCharacterPreview(3);
}
