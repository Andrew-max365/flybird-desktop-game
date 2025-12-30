#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
// mainmenu.h
class Game;   // 前向声明

class MainMenu : public QWidget {
    Q_OBJECT
public:
    //explicit MainMenu(QWidget *parent = nullptr);
    void setPendingDifficulty(int level) { pendingDifficulty = level; }
    explicit MainMenu(Game* gameWidget, QWidget *parent = nullptr);

signals:
    void startGame(int difficultyLevel); // 带参数开始游戏信号
    void showDifficultySelect(); // 显示难度选择
    void showCharacterSelect();  // 显示角色选择

private slots:
    void onStartGameClicked();
    void onDifficultySelectClicked();
    void onCharacterSelectClicked();

private:
    void setupUI();
    void applyStyle();

    Game* game;   // 保存指针
    QLabel *titleLabel;
    QPushButton *startButton;
    QPushButton *difficultyButton;
    QPushButton *characterButton;
    QPushButton *exitButton;
    int pendingDifficulty = 1;  // 默认中等难度
};

#endif // MAINMENU_H
