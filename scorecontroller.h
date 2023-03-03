#pragma once

#include <QMainWindow>
#include <QFile>
//#include <QSoundEffect>
#include <QFileInfoList>
#include <QSettings>

#include "panelorientation.h"


QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QPushButton)


class ScoreController : public QMainWindow
{
    Q_OBJECT

public:
    ScoreController(QFile *myLogFile, QWidget *parent = nullptr);
    ~ScoreController();

signals:

protected slots:
    void onButtonSpotLoopClicked();
    void onButtonSlideShowClicked();
    void onButtonSetupClicked();
    void onButtonShutdownClicked();

    void onGetPanelDirection();
    void onSetPanelDirection(PanelOrientation direction);

protected:
    bool            prepareLogFile();
    void            prepareServices();
    void            UpdateUI();
    QHBoxLayout*    CreateSpotButtons();
    void            connectButtonSignals();
    virtual void    SaveStatus();
    virtual void    GeneralSetup();
    virtual void    startSpotLoop();
    virtual void    stopSpotLoop();
    virtual void    startSlideShow();
    virtual void    stopSlideShow();

protected:
    int                   iCurrentSpot;
    QFile*                pLogFile;
//    QSoundEffect*         pButtonClick;
    QSettings*            pSettings;
    QPushButton*          pSpotButton{};
    QPushButton*          pSlideShowButton{};
    QPushButton*          pGeneralSetupButton{};
    QPushButton*          pShutdownButton{};
    QHBoxLayout*          pSpotButtonsLayout;
    enum status {
        showPanel,
        showSpots,
        showSlides,
        showCamera
    };
    status                myStatus;
};
