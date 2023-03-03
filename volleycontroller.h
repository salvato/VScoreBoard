#pragma once

#include "scorecontroller.h"
#include "generalsetuparguments.h"
#include "panelorientation.h"


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(Edit)
QT_FORWARD_DECLARE_CLASS(myLabel)
QT_FORWARD_DECLARE_CLASS(Button)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(VolleyPanel)
QT_FORWARD_DECLARE_CLASS(ClientListDialog)


class VolleyController : public ScoreController
{
    Q_OBJECT
public:
    VolleyController(QFile *myLogFile, QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);

protected:
    void          GetSettings();
    void          SaveSettings();
    void          SaveStatus();
    void          GeneralSetup();
    void          setWindowLayout();
    QGridLayout*  CreateGamePanel();
    QHBoxLayout*  CreateGameButtons();
    void          buildFontSizes();
    void          startSpotLoop();
    void          stopSpotLoop();
    void          startSlideShow();
    void          stopSlideShow();

private slots:
    void closeEvent(QCloseEvent*);
    void onTimeOutIncrement(int iTeam);
    void onTimeOutDecrement(int iTeam);
    void onSetIncrement(int iTeam);
    void onSetDecrement(int iTeam);
    void onScoreIncrement(int iTeam);
    void onScoreDecrement(int iTeam);
    void onServiceClicked(int iTeam);
    void onTeamTextChanged(QString sText, int iTeam);
    void onButtonChangeFieldClicked();
    void onButtonNewSetClicked();
    void onButtonNewGameClicked();
    void onChangePanelOrientation(PanelOrientation orientation);

private:
    void          buildControls();
    void          setEventHandlers();
    void          sendAll();

private:
    GeneralSetupArguments generalSetupArguments;
    VolleyPanel*  pVolleyPanel;
    QString       sTeam[2];
    int           iTimeout[2]{};
    int           iSet[2]{};
    int           iScore[2]{};
    int           iServizio{};
    int           lastService{};
    Edit*         pTeamName[2]{};
    Edit*         pTimeoutEdit[2]{};
    Edit*         pSetsEdit[2]{};
    Edit*         pScoreEdit[2]{};
    Button*       pTimeoutIncrement[2]{};
    Button*       pTimeoutDecrement[2]{};
    Button*       pSetsIncrement[2]{};
    Button*       pSetsDecrement[2]{};
    Button*       pScoreIncrement[2]{};
    Button*       pScoreDecrement[2]{};
    Button*       pService[2]{};
    QLabel*       pTimeoutLabel{};
    QLabel*       pSetsLabel{};
    QLabel*       pServiceLabel{};
    QLabel*       pScoreLabel{};

    QPushButton*  pNewSetButton{};
    QPushButton*  pNewGameButton{};
    QPushButton  *changeFieldButton{};
    bool          bFontBuilt;
    QPalette           panelPalette;
    QLinearGradient    panelGradient;
    QBrush             panelBrush;
};

