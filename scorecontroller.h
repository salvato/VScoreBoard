#pragma once

#include <QMainWindow>
#include <QFile>
#include <QSoundEffect>
#include <QFileInfoList>
#include <QSettings>

#include "panelorientation.h"
#include "generalsetuparguments.h"


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
    void onButtonStartStopSpotLoopClicked();
    void onButtonStartStopSlideShowClicked();
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
    GeneralSetupArguments generalSetupArguments;
    QFile*                pLogFile;
    QSoundEffect*         pButtonClick;
    int                   iCurrentSpot;
    QSettings*            pSettings;
    QPushButton*          pSpotButton{};
    QPushButton*          pSlideShowButton{};
    QPushButton*          generalSetupButton{};
    QPushButton*          shutdownButton{};
    QHBoxLayout*          pSpotButtonsLayout;
    enum status {
        showPanel,
        showSpots,
        showSlides,
        showCamera
    };
    status                myStatus;
};
