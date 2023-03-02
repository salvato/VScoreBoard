#pragma once

#include <QMainWindow>
#include <QList>
#include <QVector>
#include <QFileInfoList>
#include <QUrl>

#include "scorepanel.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(TimeoutWindow)

class VolleyPanel : public ScorePanel
{
    Q_OBJECT

public:
    VolleyPanel(QFile *myLogFile, QWidget *parent = nullptr);
    ~VolleyPanel();
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void setTeam(int iTeam, QString sTeamName);
    void setScore(int iTeam, int iScore);
    void setSets(int iTeam, int iSets);
    void setTimeout(int iTeam, int iTimeout);
    void startTimeout(int iTimeoutDuration);
    void stopTimeout();
    void setServizio(int iServizio);
    void setMirrored(bool isPanelMirrored);
    bool getMirrored();

private:
    QLabel            *team[2];
    QLabel            *score[2];
    QLabel            *scoreLabel;
    QLabel            *set[2];
    QLabel            *setLabel;
    QLabel            *servizio[2];
    QLabel            *timeout[2];
    QLabel            *timeoutLabel;
    QString            sFontName;
    int                fontWeight;
    QPalette           panelPalette;
    QLinearGradient    panelGradient;
    QBrush             panelBrush;
    int                iServizio;
    int                iTimeoutFontSize;
    int                iSetFontSize;
    int                iScoreFontSize;
    int                iTeamFontSize;
    int                iLabelsFontSize;
    int                maxTeamNameLen;
    QPixmap*           pPixmapService;

    void               createPanelElements();
    QGridLayout*       createPanel();
    TimeoutWindow     *pTimeoutWindow;

private slots:
    void onTimeoutDone();
};
