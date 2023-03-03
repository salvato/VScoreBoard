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
    void setLogo(int iTeam, QString sFileLogo);

private:
    QLabel*           pTeam[2];
    QLabel*           pScore[2];
    QLabel*           pScoreLabel;
    QLabel*           pSet[2];
    QLabel*           pSetLabel;
    QLabel*           pServizio[2];
    QLabel*           pTimeout[2];
    QLabel*           pTimeoutLabel;
    QLabel*           logoLabel[2];
    QPixmap*          pPixmapLogo[2];

    QString           sFontName;
    int               fontWeight;
    QPalette          panelPalette;
    QLinearGradient   panelGradient;
    QBrush            panelBrush;
    int               iServizio;
    int               iTimeoutFontSize;
    int               iSetFontSize;
    int               iScoreFontSize;
    int               iTeamFontSize;
    int               iLabelsFontSize;
    int               maxTeamNameLen;
    QPixmap*          pPixmapService;
//    QPixmap*          pPixmapLeftTop;
//    QPixmap*          pPixmapRightTop;
    void              createPanelElements();
    QGridLayout*      createPanel();
    TimeoutWindow*    pTimeoutWindow;

private slots:
    void onTimeoutDone();
};
