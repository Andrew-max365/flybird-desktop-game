#ifndef CHARACTERSELECTION_H
#define CHARACTERSELECTION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

class CharacterSelection : public QWidget {
    Q_OBJECT
public:
    explicit CharacterSelection(QWidget *parent = nullptr);

    // 获取角色参数
    struct CharacterInfo {
        QString name;
        QString imagePath;
        int size;
        qreal gravity;
        qreal lift;
        QString description;
    };

    static CharacterInfo getCharacterInfo(int characterId);

signals:
    void backToMainMenu();                      // 返回主菜单信号
    void characterSelected(const CharacterInfo& info);  // 角色选择信号（包含完整信息）

private slots:
    void onBackClicked();
    void onCharacter1Clicked();
    void onCharacter2Clicked();
    void onCharacter3Clicked();

private:
    void setupUI();
    void applyStyle();
    void updateCharacterPreview(int characterId);

    // 角色预览区域
    QLabel *characterPreview;
    QLabel *characterStats;

    QLabel *titleLabel;
    QPushButton *character1Button;
    QPushButton *character2Button;
    QPushButton *character3Button;
    QPushButton *backButton;
    QPushButton *confirmButton;

    int currentSelectedId;

    // 角色信息
    static QMap<int, CharacterInfo> characterMap;
};

#endif // CHARACTERSELECTION_H
