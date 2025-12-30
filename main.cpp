#include <QApplication>
#include "game.h"
#include "difficultyselection.h"
#include "mainmenu.h"
#include "characterselection.h"
#include "main.h"  // 包含全局配置

// 全局配置变量定义
GlobalCharacterConfig g_characterConfig;

// 全局配置管理函数实现
void initGlobalConfig() {
    g_characterConfig.resetToDefault();
}

GlobalCharacterConfig& getGlobalCharacterConfig() {
    return g_characterConfig;
}

void setGlobalCharacterConfig(const GlobalCharacterConfig& config) {
    g_characterConfig = config;
}

void resetGlobalCharacterConfig() {
    g_characterConfig.resetToDefault();
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // 初始化全局配置
    initGlobalConfig();

    Game game;

    // 创建主菜单
    MainMenu mainMenu(&game);
    mainMenu.show();

    // 创建其他界面（但不显示）
    DifficultySelection difficultySelection;
    CharacterSelection characterSelection;

    // 隐藏其他窗口
    difficultySelection.hide();
    characterSelection.hide();
    game.hide();

    // 主菜单信号连接
    QObject::connect(&mainMenu, &MainMenu::startGame,
                     &game, [&game, &mainMenu](int level){
                        GlobalCharacterConfig config = getGlobalCharacterConfig();
                        game.setCharacterFromConfig(config);
                         game.setDifficulty(level);
                         game.show();
                         mainMenu.hide();
                     });

    QObject::connect(&mainMenu, &MainMenu::showDifficultySelect,
                     &difficultySelection, [&difficultySelection, &mainMenu]() {
                         difficultySelection.show();
                         mainMenu.hide();
                     });

    QObject::connect(&mainMenu, &MainMenu::showCharacterSelect,
                     &characterSelection, [&characterSelection, &mainMenu]() {
                         characterSelection.show();
                         mainMenu.hide();
                     });


    QObject::connect(&difficultySelection, &DifficultySelection::difficultySelected,
                     &mainMenu, [&mainMenu](int level){
                         mainMenu.setPendingDifficulty(level);
                         mainMenu.show();
                         //difficultySelection.hide();
                     });

    // 角色选择信号连接
    QObject::connect(&characterSelection, &CharacterSelection::characterSelected,
                     [&mainMenu, &characterSelection](const CharacterSelection::CharacterInfo& info) {
                         // 保存角色配置到全局
                         GlobalCharacterConfig config;
                         config.imagePath = info.imagePath;
                         config.size = info.size;
                         config.gravity = info.gravity;
                         config.lift = info.lift;
                         config.name = info.name;

                         setGlobalCharacterConfig(config);

                         qDebug() << "角色配置已保存到全局：" << config.toString();

                         // 返回主菜单
                         mainMenu.show();
                         characterSelection.hide();
                     });

    // 角色选择返回主菜单
    QObject::connect(&characterSelection, &CharacterSelection::backToMainMenu,
                     &mainMenu, [&mainMenu, &characterSelection]() {
                         mainMenu.show();
                         characterSelection.hide();
                     });

    // 游戏结束返回主菜单
    QObject::connect(&game, &Game::closed,
                     &mainMenu, [&mainMenu, &game]() {
                         game.hide();
                         mainMenu.show();
                     });

    return a.exec();
}
