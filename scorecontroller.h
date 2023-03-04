#pragma once

#include <QMainWindow>
#include <QFile>
#include <QFileInfoList>
#include <QSettings>
#include <QResizeEvent>


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

protected:
    void            resizeEvent(QResizeEvent* pEvent);
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
    QFile*                pLogFile;
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
