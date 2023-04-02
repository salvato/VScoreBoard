#pragma once

#include <QWidget>
#include <QLabel>

#include "racewidget.h"

class RaceWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RaceWindow(QWidget *parent = nullptr);

signals:
    void raceDone();

public:
    void updateScore(int team0Score, int team1Score, int iSet);
    void updateLabel(int iTeam, QString sLabel);
    void resetScore(int iSet);
    void resetAll();
    void startRace(int iSet);

public slots:
    void onNewScore(int s0, int s1);
    void onTimeToStartRace();
    //void closeEvent(QCloseEvent*) override;

private:
    QPalette        panelPalette;
    QLinearGradient panelGradient;
    QBrush          panelBrush;

    RaceWidget* pRaceWidget;
    QString sTeamName[2];
    QVector<QVector2D> score[5];
    int maxScore[5];
    int indexScore;
    int iCurrentSet;
    QLabel labelTeam0;
    QLabel labelTeam1;
    QLabel labelIcon0;
    QLabel labelIcon1;
    QLabel labelScore0;
    QLabel labelScore1;

    QTimer timerStart;
};
