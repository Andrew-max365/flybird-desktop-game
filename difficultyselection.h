#ifndef DIFFICULTYSELECTION_H
#define DIFFICULTYSELECTION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class DifficultySelection : public QWidget {
    Q_OBJECT
public:
    explicit DifficultySelection(QWidget *parent = nullptr);

signals:
    void difficultySelected(int level);  // 0: 简单, 1: 中等, 2: 困难

private slots:
    void onEasyClicked();
    void onMediumClicked();
    void onHardClicked();

private:
    void setupUI();
    void applyStyle();

    QLabel *titleLabel;
    QPushButton *easyButton;
    QPushButton *mediumButton;
    QPushButton *hardButton;
};

#endif // DIFFICULTYSELECTION_H
