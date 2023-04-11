/*
 *
Copyright (C) 2023  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include <QSettings>
#include <QGridLayout>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include "volleycontroller.h"
#include "generalsetupdialog.h"
#include "edit.h"
#include "button.h"
#include "volleypanel.h"
#include "utility.h"
#include "chartwindow.h"
#include "racewindow.h"


VolleyController::VolleyController(QFile *myLogFile, QWidget *parent)
    : ScoreController(myLogFile, parent)
    , pVolleyPanel(new VolleyPanel(myLogFile))
    , bFontBuilt(false)
    , pCharts(nullptr)
    , pScoreFile(nullptr)
    , pRaceWindow(nullptr)
{
    setWindowTitle("Score Controller - © Gabriele Salvato (2023)");
    setWindowIcon(QIcon(":/Logo.ico"));

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);

    panelPalette.setColor(QPalette::WindowText,      Qt::yellow);
    panelPalette.setColor(QPalette::Base,            Qt::black);
    panelPalette.setColor(QPalette::AlternateBase,   Qt::blue);
    panelPalette.setColor(QPalette::Text,            Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,      Qt::white);
    panelPalette.setColor(QPalette::HighlightedText, Qt::gray);
    panelPalette.setColor(QPalette::Highlight,       Qt::transparent);

    setPalette(panelPalette);

    GetSettings();

    buildControls();
    setWindowLayout();

    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pService[iServizio ? 0 : 1]->setFocus();

    prepareScoreFile();

    pVolleyPanel->showFullScreen();

    pCharts = new ChartWindow();
    pCharts->updateLabel(0, gsArgs.sTeam[0]);
    pCharts->updateLabel(1, gsArgs.sTeam[1]);

    pRaceWindow = new RaceWindow();
    pRaceWindow->updateLabel(0, gsArgs.sTeam[0]);
    pRaceWindow->updateLabel(1, gsArgs.sTeam[1]);

    setEventHandlers();
}


void
VolleyController::closeEvent(QCloseEvent *event) {
    if(pScoreFile) {
        pScoreFile->close();
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    SaveSettings();
    if(pVolleyPanel) delete pVolleyPanel;
    if(pCharts) delete pCharts;
    if(pRaceWindow) delete pRaceWindow;
    ScoreController::closeEvent(event);
    event->accept();
}


void
VolleyController::resizeEvent(QResizeEvent *event) {
    QList<QScreen*> screens = QApplication::screens();
    QRect screenRect = screens.at(0)->geometry();
    QRect myRect = frameGeometry();
    int x0 = (screenRect.width() - myRect.width())/2;
    int y0 = (screenRect.height()-myRect.height())/2;
    move(x0,y0);
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


bool
VolleyController::prepareScoreFile() {
    QString sPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
    sScoreFileName = "Volley_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm");
    sScoreFileName = sPath + sScoreFileName + ".txt";
    QFileInfo checkFile(sScoreFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(sScoreFileName+QString(".bkp"));
        renamed.rename(sScoreFileName, sScoreFileName+QString(".bkp"));
    }
    if(pScoreFile) {
        pScoreFile->close();
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    pScoreFile = new QFile(sScoreFileName);
    if (!pScoreFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(Q_NULLPTR, "Segnapunti Volley",
                                 QString("Impossibile aprire il file %1: %2.")
                                 .arg(sScoreFileName, pScoreFile->errorString()));
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    return true;
}


void
VolleyController::logScore() {
    // Set, Team0Name, Team0Score, Team1Name, Team1Score, Time
    QString sMessage = QString("%1, %2, %3, %4, %5, %6\n")
                               .arg(iSet[0]+iSet[1]+1)
                               .arg(pTeamName[0]->text())
                               .arg(iScore[0])
                               .arg(pTeamName[1]->text())
                               .arg(iScore[1])
                               .arg(QTime::currentTime().toString("hh:mm:ss"));
    if(pScoreFile) {
        if(pScoreFile->isOpen()) {
            pScoreFile->write(sMessage.toLatin1()); // toLatin1() --> converted to ASCII
            pScoreFile->flush();
        }
        else
            qCritical() << sMessage;
    }
    else
        qCritical() << sMessage;
}


void
VolleyController::updateStatistics() {
    if(pCharts)
        pCharts->updateScore(iScore[0], iScore[1], iSet[0]+iSet[1]);
    if(pRaceWindow)
        pRaceWindow->updateScore(iScore[0], iScore[1], iSet[0]+iSet[1]);
}


void
VolleyController::GeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&gsArgs);
    connect(pGeneralSetupDialog, SIGNAL(changeOrientation(PanelOrientation)),
            this, SLOT(onChangePanelOrientation(PanelOrientation)));
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        if(!gsArgs.sSlideDir.endsWith(QString("/")))
            gsArgs.sSlideDir+= QString("/");
        QDir slideDir(gsArgs.sSlideDir);
        if(!slideDir.exists()) {
            gsArgs.sSlideDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
        }
        if(!gsArgs.sSpotDir.endsWith(QString("/")))
            gsArgs.sSpotDir+= QString("/");
        QDir spotDir(gsArgs.sSpotDir);
        if(!spotDir.exists()) {
            gsArgs.sSpotDir = QStandardPaths::displayName(QStandardPaths::MoviesLocation);
        }
        SaveSettings();
        sendAll();
    }
    delete pGeneralSetupDialog;
    pGeneralSetupDialog = nullptr;
}


void
VolleyController::buildFontSizes() {
    QFont font;
    int iFontSize;
    int hMargin, vMargin;
    QMargins margins;

    font = pTeamName[0]->font();
    font.setCapitalization(QFont::Capitalize);
    margins = pTeamName[0]->contentsMargins();
    vMargin = margins.bottom() + margins.top();
    hMargin = margins.left() + margins.right();
    iFontSize = qMin((pTeamName[0]->width()/pTeamName[0]->maxLength())-2*hMargin,
            pTeamName[0]->height()-vMargin);

    font.setPixelSize(iFontSize);

    font.setPixelSize(iFontSize*0.75);
    pTimeoutLabel->setFont(font);
    pSetsLabel->setFont(font);
    pServiceLabel->setFont(font);
    pScoreLabel->setFont(font);

    font.setWeight(QFont::Black);

    font.setPixelSize(iFontSize);
    pTeamName[0]->setFont(font);
    pTeamName[1]->setFont(font);
    pSetsEdit[0]->setFont(font);
    pSetsEdit[1]->setFont(font);
    pTimeoutEdit[0]->setFont(font);
    pTimeoutEdit[1]->setFont(font);

    font.setPixelSize(2*iFontSize);
    pScoreEdit[0]->setFont(font);
    pScoreEdit[1]->setFont(font);
}



void
VolleyController::setWindowLayout() {
    QWidget *widget = new QWidget();
    auto *mainLayout = new QGridLayout();

    int gamePanelWidth  = 15;
    int gamePanelHeigth =  8;

    mainLayout->addLayout(CreateGamePanel(),
                          0,
                          0,
                          gamePanelHeigth,
                          gamePanelWidth);

    mainLayout->addLayout(CreateGameButtons(),
                          gamePanelHeigth,
                          0,
                          2,
                          8);

    mainLayout->addLayout(pSpotButtonsLayout,
                          gamePanelHeigth,
                          8,
                          2,
                          gamePanelWidth-5);

    widget->setLayout(mainLayout);
    setCentralWidget(widget);
    setTabOrder(pTeamName[0], pTeamName[1]);
}


QGridLayout*
VolleyController::CreateGamePanel() {
    auto* gamePanel = new QGridLayout();
    // Team
    int iRow;
    for(int iTeam=0; iTeam<2; iTeam++) {
        // Matrice x righe e 8 colonne
        iRow = 0;
        gamePanel->addWidget(pTeamName[iTeam], iRow, iTeam*4, 1, 4);
        int iCol = iTeam*5;
        iRow += 1;
        gamePanel->addWidget(pScoreDecrement[iTeam], iRow, iCol,   2, 1, Qt::AlignRight);
        gamePanel->addWidget(pScoreEdit[iTeam],      iRow, iCol+1, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pScoreIncrement[iTeam], iRow, iCol+2, 2, 1, Qt::AlignLeft);
        iRow += 2;
        gamePanel->addWidget(pService[iTeam],   iRow, iCol, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
        iRow += 1;
        gamePanel->addWidget(pSetsDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(pSetsEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pSetsIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
        iRow += 1;
        gamePanel->addWidget(pTimeoutDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(pTimeoutEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pTimeoutIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
    }
    iRow += 1;
    QFrame* myFrame = new QFrame();
    myFrame->setFrameShape(QFrame::HLine);
    gamePanel->addWidget(myFrame, iRow, 0, 1, 10);

    iRow = 1;
    gamePanel->addWidget(pScoreLabel,   iRow, 3, 2, 2);
    iRow += 2;
    gamePanel->addWidget(pServiceLabel, iRow, 3, 1, 2);
    iRow += 1;
    gamePanel->addWidget(pSetsLabel,    iRow, 3, 1, 2);
    iRow += 1;
    gamePanel->addWidget(pTimeoutLabel, iRow, 3, 1, 2);
    //    iRow += 1;

    return gamePanel;
}


QHBoxLayout*
VolleyController::CreateGameButtons() {
    auto* gameButtonLayout = new QHBoxLayout();
    QSize iconSize = QSize(48,48);

    QPixmap* pPixmap = new QPixmap(":/buttonIcons/ExchangeVolleyField.png");
    pChangeFieldButton = new QPushButton(QIcon(*pPixmap), "");
    pChangeFieldButton->setIconSize(iconSize);
    pChangeFieldButton->setFlat(true);
    pChangeFieldButton->setToolTip("Inverti Campo");

    pPixmap->load(":/buttonIcons/New-Game-Volley.png");
    pNewGameButton = new QPushButton(QIcon(*pPixmap), "");
    pNewGameButton->setIconSize(iconSize);
    pNewGameButton->setFlat(true);
    pNewGameButton->setToolTip("Nuova Partita");

    pPixmap->load(":/buttonIcons/New-Set-Volley.png");
    pNewSetButton  = new QPushButton(*pPixmap, "");
    pNewSetButton->setIconSize(iconSize);
    pNewSetButton->setFlat(true);
    pNewSetButton->setToolTip("Nuovo Set");

    pPixmap->load(":/buttonIcons/plot.png");
    pStatisticButton  = new QPushButton(QIcon(*pPixmap), "");
    pStatisticButton->setIconSize(iconSize);
    pStatisticButton->setFlat(true);
    pStatisticButton->setToolTip("Statistiche");

    delete pPixmap;

    gameButtonLayout->addWidget(pNewGameButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pNewSetButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pChangeFieldButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pStatisticButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
VolleyController::GetSettings() {
    gsArgs.maxTimeout           = pSettings->value("volley/maxTimeout", 2).toInt();
    gsArgs.maxSet               = pSettings->value("volley/maxSet", 3).toInt();
    gsArgs.iTimeoutDuration     = pSettings->value("volley/TimeoutDuration", 30).toInt();
    gsArgs.sSlideDir            = pSettings->value("directories/slides", gsArgs.sSlideDir).toString();
    gsArgs.sSpotDir             = pSettings->value("directories/spots",  gsArgs.sSpotDir).toString();
    gsArgs.isPanelMirrored      = pSettings->value("panel/orientation",  true).toBool();
    gsArgs.sTeamLogoFilePath[0] = pSettings->value("panel/logo0", ":/Logo_UniMe.png").toString();
    gsArgs.sTeamLogoFilePath[1] = pSettings->value("panel/logo1", ":/Logo_SSD_UniMe.png").toString();
    gsArgs.sTeam[0]             = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    gsArgs.sTeam[1]             = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();

    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iSet[0]     = pSettings->value("team1/sets", 0).toInt();
    iSet[1]     = pSettings->value("team2/sets", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iServizio   = pSettings->value("set/service", 0).toInt();
    lastService = pSettings->value("set/lastservice", 0).toInt();

    // Check Stored Values vs Maximum Values
    for(int i=0; i<2; i++) {
        if(iTimeout[i] > gsArgs.maxTimeout)
            iTimeout[i] = gsArgs.maxTimeout;
        if(iSet[i] > gsArgs.maxSet)
            iSet[i] = gsArgs.maxSet;
    }

    sendAll();
}


void
VolleyController::sendAll() {
    for(int i=0; i<2; i++) {
        pVolleyPanel->setTeam(i, gsArgs.sTeam[i]);
        pVolleyPanel->setTimeout(i, iTimeout[i]);
        pVolleyPanel->setSets(i, iSet[i]);
        pVolleyPanel->setScore(i, iScore[i]);
    }
    pVolleyPanel->setLogo(0, gsArgs.sTeamLogoFilePath[0]);
    pVolleyPanel->setLogo(1, gsArgs.sTeamLogoFilePath[1]);
    pVolleyPanel->setServizio(iServizio);
    pVolleyPanel->setMirrored(gsArgs.isPanelMirrored);
}


void
VolleyController::SaveStatus() {
    // Save Present Game Values
    pSettings->setValue("team1/name", gsArgs.sTeam[0]);
    pSettings->setValue("team2/name", gsArgs.sTeam[1]);
    pSettings->setValue("team1/timeouts", iTimeout[0]);
    pSettings->setValue("team2/timeouts", iTimeout[1]);
    pSettings->setValue("team1/sets", iSet[0]);
    pSettings->setValue("team2/sets", iSet[1]);
    pSettings->setValue("team1/score", iScore[0]);
    pSettings->setValue("team2/score", iScore[1]);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
}


void
VolleyController::SaveSettings() { // Save General Setup Values
    pSettings->setValue("directories/slides",     gsArgs.sSlideDir);
    pSettings->setValue("directories/spots",      gsArgs.sSpotDir);
    pSettings->setValue("volley/maxTimeout",      gsArgs.maxTimeout);
    pSettings->setValue("volley/maxSet",          gsArgs.maxSet);
    pSettings->setValue("volley/TimeoutDuration", gsArgs.iTimeoutDuration);
    pSettings->setValue("panel/orientation",      gsArgs.isPanelMirrored);
    pSettings->setValue("panel/logo0",            gsArgs.sTeamLogoFilePath[0]);
    pSettings->setValue("panel/logo1",            gsArgs.sTeamLogoFilePath[1]);
}


void
VolleyController::buildControls() {
    QString sString;

    QPixmap plusPixmap, minusPixmap;
    QIcon plusButtonIcon, minusButtonIcon;
    plusPixmap.load(":/buttonIcons/Plus.png");
    plusButtonIcon.addPixmap(plusPixmap);
    minusPixmap.load(":/buttonIcons/Minus.png");
    minusButtonIcon.addPixmap(minusPixmap);

    QPalette pal = panelPalette;
    pal.setColor(QPalette::Text, Qt::white);
    for(int iTeam=0; iTeam<2; iTeam++){
        // Teams
        pTeamName[iTeam] = new Edit(gsArgs.sTeam[iTeam], iTeam);
        pTeamName[iTeam]->setAlignment(Qt::AlignHCenter);
        pTeamName[iTeam]->setMaxLength(MAX_NAMELENGTH);
        pal.setColor(QPalette::Text, Qt::white);
        pTeamName[iTeam]->setPalette(pal);
        // Timeout
        sString = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam] = new Edit(sString, iTeam);
        pTimeoutEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pTimeoutEdit[iTeam]->setMaxLength(1);
        pal.setColor(QPalette::Text, Qt::yellow);
        pTimeoutEdit[iTeam]->setPalette(pal);
        pTimeoutEdit[iTeam]->setReadOnly(true);
        // Timeout buttons
        pTimeoutIncrement[iTeam] = new Button("", iTeam);
        pTimeoutIncrement[iTeam]->setIcon(plusButtonIcon);
        pTimeoutIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pTimeoutDecrement[iTeam] = new Button("", iTeam);
        pTimeoutDecrement[iTeam]->setIcon(minusButtonIcon);
        pTimeoutDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iTimeout[iTeam] == 0)
            pTimeoutDecrement[iTeam]->setEnabled(false);
        if(iTimeout[iTeam] == gsArgs.maxTimeout) {
            pTimeoutIncrement[iTeam]->setEnabled(false);
            pTimeoutEdit[iTeam]->setStyleSheet("background:rgba(0, 0, 0, 0);color:red; border: none");
        }
        // Sets
        sString = QString("%1").arg(iSet[iTeam], 1);
        pSetsEdit[iTeam] = new Edit(sString, iTeam);
        pSetsEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pSetsEdit[iTeam]->setMaxLength(1);
        pSetsEdit[iTeam]->setPalette(pal);
        pSetsEdit[iTeam]->setReadOnly(true);
        // Set buttons
        pSetsIncrement[iTeam] = new Button("", iTeam);
        pSetsIncrement[iTeam]->setIcon(plusButtonIcon);
        pSetsIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pSetsDecrement[iTeam] = new Button("", iTeam);
        pSetsDecrement[iTeam]->setIcon(minusButtonIcon);
        pSetsDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iSet[iTeam] == 0)
            pSetsDecrement[iTeam]->setEnabled(false);
        if(iSet[iTeam] == gsArgs.maxSet)
            pSetsIncrement[iTeam]->setEnabled(false);
        // Service
        QPixmap pixmap(QString(":/ball%1.png").arg(iTeam));
        QIcon ButtonIcon(pixmap);
        pService[iTeam] = new Button("", iTeam);
        pService[iTeam]->setIcon(ButtonIcon);
        auto const rec = QApplication::primaryScreen()->availableSize();
        auto const height = rec.height();
        pService[iTeam]->setIconSize(QSize(height/16,height/16));
        pService[iTeam]->setCheckable(true);
        pService[iTeam]->setStyleSheet("QPushButton:checked { background-color: rgb(128, 128, 255); border:none }");
        // Score
        pScoreLabel = new QLabel(tr("Punti"));
        pScoreLabel->setAlignment(Qt::AlignRight|Qt::AlignHCenter);
        sString = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam] = new Edit(sString, iTeam);
        pScoreEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pScoreEdit[iTeam]->setMaxLength(2);
        pScoreEdit[iTeam]->setPalette(pal);
        pScoreEdit[iTeam]->setReadOnly(true);
        // Score buttons
        pScoreIncrement[iTeam] = new Button("", iTeam);
        pScoreIncrement[iTeam]->setIcon(plusButtonIcon);
        pScoreIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pScoreDecrement[iTeam] = new Button("", iTeam);
        pScoreDecrement[iTeam]->setIcon(minusButtonIcon);
        pScoreDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iScore[iTeam] == 0)
            pScoreDecrement[iTeam]->setEnabled(false);
    }
    // Timeout
    pTimeoutLabel = new QLabel(tr("Timeout"));
    pTimeoutLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Set
    pSetsLabel = new QLabel(tr("Set"));
    pSetsLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Service
    pServiceLabel = new QLabel(tr("Servizio"));
    pServiceLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Score
    pScoreLabel = new QLabel(tr("Punti"));
    pScoreLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}


void
VolleyController::setEventHandlers() {
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(pTeamName[iTeam], SIGNAL(teamTextChanged(QString,int)),
                this, SLOT(onTeamTextChanged(QString,int)));
        connect(pTimeoutIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutIncrement(int)));
        connect(pTimeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(pSetsIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetIncrement(int)));
        connect(pSetsDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetDecrement(int)));
        connect(pService[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onServiceClicked(int)));
        connect(pScoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(pScoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
    }
    // New Set
    connect(pNewSetButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewSetClicked()));
    // New Game
    connect(pNewGameButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewGameClicked()));
    // Exchange Field Position
    connect(pChangeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
    // Show Statistics
    connect(pStatisticButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonStatisticsClicked()));
    // Statistics Window Signal
    connect(pCharts, SIGNAL(done()),
            this, SLOT(onStatisticsDone()));
    connect(pRaceWindow, SIGNAL(raceDone()),
            this, SLOT(onRaceDone()));
/*
 Keypress Sound
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(pTimeoutIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pTimeoutDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pSetsIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pSetsDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pService[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pScoreIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pScoreDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
    }
    connect(pNewSetButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    connect(pNewGameButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    connect(changeFieldButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
*/
}


// =========================
// Event management routines
// =========================

void
VolleyController::onTimeOutIncrement(int iTeam) {
    iTimeout[iTeam]++;
    if(iTimeout[iTeam] >= gsArgs.maxTimeout) {
        pTimeoutIncrement[iTeam]->setEnabled(false);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:red; border: none");
    }
    pTimeoutDecrement[iTeam]->setEnabled(true);
    pVolleyPanel->setTimeout(iTeam, iTimeout[iTeam]);
    if(gsArgs.iTimeoutDuration > 0) {
        pVolleyPanel->startTimeout(gsArgs.iTimeoutDuration);
    }
    QString sText = QString("%1").arg(iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setFocus(); // Per evitare che il focus vada all'edit delle squadre
}


void
VolleyController::onTimeOutDecrement(int iTeam) {
    iTimeout[iTeam]--;
    if(iTimeout[iTeam] == 0) {
        pTimeoutDecrement[iTeam]->setEnabled(false);
    }
    pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
    pTimeoutIncrement[iTeam]->setEnabled(true);
    pVolleyPanel->setTimeout(iTeam, iTimeout[iTeam]);
    pVolleyPanel->stopTimeout();
    QString sText;
    sText = QString("%1").arg(iTimeout[iTeam], 1);
    pTimeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
}


void
VolleyController::onSetIncrement(int iTeam) {
    iSet[iTeam]++;
    pSetsDecrement[iTeam]->setEnabled(true);
    if(iSet[iTeam] == gsArgs.maxSet) {
        pSetsIncrement[iTeam]->setEnabled(false);
    }
    pVolleyPanel->setSets(iTeam, iSet[iTeam]);
    QString sText;
    sText = QString("%1").arg(iSet[iTeam], 1);
    pSetsEdit[iTeam]->setText(sText);
    sText = QString("team%1/sets").arg(iTeam+1, 1);
    pSettings->setValue(sText, iSet[iTeam]);
}


void
VolleyController::onSetDecrement(int iTeam) {
    iSet[iTeam]--;
    pSetsIncrement[iTeam]->setEnabled(true);
    if(iSet[iTeam] == 0) {
        pSetsDecrement[iTeam]->setEnabled(false);
    }
    pVolleyPanel->setSets(iTeam, iSet[iTeam]);
    QString sText;
    sText = QString("%1").arg(iSet[iTeam], 1);
    pSetsEdit[iTeam]->setText(sText);
    sText = QString("team%1/sets").arg(iTeam+1, 1);
    pSettings->setValue(sText, iSet[iTeam]);
}


void
VolleyController::onServiceClicked(int iTeam) {
    iServizio = iTeam;
    lastService = iServizio;
    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pVolleyPanel->setServizio(iServizio);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
}


void
VolleyController::onScoreIncrement(int iTeam) {
    iScore[iTeam]++;
    pScoreDecrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] > 98) {
        pScoreIncrement[iTeam]->setEnabled(false);
    }
    pVolleyPanel->setScore(iTeam, iScore[iTeam]);
    lastService = iServizio;
    iServizio = iTeam;
    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pVolleyPanel->setServizio(iServizio);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    pScoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
    logScore();
    updateStatistics();
//    bool bEndSet;
//    if(iSet[0]+iSet[1] > 4)
//        bEndSet = ((iScore[0] > 14) || (iScore[1] > 14)) &&
//                  (std::abs(iScore[0]-iScore[1]) > 1);
//    else
//        bEndSet = ((iScore[0] > 24) || (iScore[1] > 24)) &&
//                  (std::abs(iScore[0]-iScore[1]) > 1);
//    if(bEndSet) {
//    }
}


void
VolleyController::onScoreDecrement(int iTeam) {
    iScore[iTeam]--;
    pScoreIncrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] == 0) {
        pScoreDecrement[iTeam]->setEnabled(false);
    }
    pVolleyPanel->setScore(iTeam, iScore[iTeam]);
    iServizio = lastService;
    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pVolleyPanel->setServizio(iServizio);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    pScoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
    logScore();
    updateStatistics();
}


void
VolleyController::onTeamTextChanged(QString sText, int iTeam) {
    gsArgs.sTeam[iTeam] = sText;
    pVolleyPanel->setTeam(iTeam, gsArgs.sTeam[iTeam]);
    sText = QString("team%1/name").arg(iTeam+1, 1);
    pSettings->setValue(sText, gsArgs.sTeam[iTeam]);
    pCharts->updateLabel(iTeam, gsArgs.sTeam[iTeam]);
    pRaceWindow->updateLabel(iTeam, gsArgs.sTeam[iTeam]);
}


void
VolleyController::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = gsArgs.sTeam[0];
    gsArgs.sTeam[0] = gsArgs.sTeam[1];
    gsArgs.sTeam[1] = sText;
    pTeamName[0]->setText(gsArgs.sTeam[0]);
    pTeamName[1]->setText(gsArgs.sTeam[1]);

    sText = gsArgs.sTeamLogoFilePath[0];
    gsArgs.sTeamLogoFilePath[0] = gsArgs.sTeamLogoFilePath[1];
    gsArgs.sTeamLogoFilePath[1] = sText;

    int iVal = iSet[0];
    iSet[0] = iSet[1];
    iSet[1] = iVal;
    sText = QString("%1").arg(iSet[0], 1);
    pSetsEdit[0]->setText(sText);
    sText = QString("%1").arg(iSet[1], 1);
    pSetsEdit[1]->setText(sText);

    iVal = iScore[0];
    iScore[0] = iScore[1];
    iScore[1] = iVal;
    sText = QString("%1").arg(iScore[0], 2);
    pScoreEdit[0]->setText(sText);
    sText = QString("%1").arg(iScore[1], 2);
    pScoreEdit[1]->setText(sText);

    iVal = iTimeout[0];
    iTimeout[0] = iTimeout[1];
    iTimeout[1] = iVal;
    sText = QString("%1").arg(iTimeout[0]);
    pTimeoutEdit[0]->setText(sText);
    sText = QString("%1").arg(iTimeout[1]);
    pTimeoutEdit[1]->setText(sText);

    iServizio = 1 - iServizio;
    lastService = 1 -lastService;

    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);

    for(int iTeam=0; iTeam<2; iTeam++) {
        pScoreDecrement[iTeam]->setEnabled(true);
        pScoreIncrement[iTeam]->setEnabled(true);
        if(iScore[iTeam] == 0) {
            pScoreDecrement[iTeam]->setEnabled(false);
        }
        if(iScore[iTeam] > 98) {
            pScoreIncrement[iTeam]->setEnabled(false);
        }

        pSetsDecrement[iTeam]->setEnabled(true);
        pSetsIncrement[iTeam]->setEnabled(true);
        if(iSet[iTeam] == 0) {
            pSetsDecrement[iTeam]->setEnabled(false);
        }
        if(iSet[iTeam] == gsArgs.maxSet) {
            pSetsIncrement[iTeam]->setEnabled(false);
        }

        pTimeoutIncrement[iTeam]->setEnabled(true);
        pTimeoutDecrement[iTeam]->setEnabled(true);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        if(iTimeout[iTeam] == gsArgs.maxTimeout) {
            pTimeoutIncrement[iTeam]->setEnabled(false);
            pTimeoutEdit[iTeam]->setStyleSheet("background:rgba(0, 0, 0, 0);color:white; border: none");
        }
        if(iTimeout[iTeam] == 0) {
            pTimeoutDecrement[iTeam]->setEnabled(false);
        }
    }
    sendAll();
    SaveStatus();
}


void
VolleyController::onButtonNewSetClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Vuoi davvero iniziare un nuovo Set ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    // Exchange team's order in the field
    QString sText = gsArgs.sTeam[0];
    gsArgs.sTeam[0] = gsArgs.sTeam[1];
    gsArgs.sTeam[1] = sText;
    pTeamName[0]->setText(gsArgs.sTeam[0]);
    pTeamName[1]->setText(gsArgs.sTeam[1]);

    sText = gsArgs.sTeamLogoFilePath[0];
    gsArgs.sTeamLogoFilePath[0] = gsArgs.sTeamLogoFilePath[1];
    gsArgs.sTeamLogoFilePath[1] = sText;

    int iVal = iSet[0];
    iSet[0] = iSet[1];
    iSet[1] = iVal;
    sText = QString("%1").arg(iSet[0], 1);
    pSetsEdit[0]->setText(sText);
    sText = QString("%1").arg(iSet[1], 1);
    pSetsEdit[1]->setText(sText);
    for(int iTeam=0; iTeam<2; iTeam++) {
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam]->setText(sText);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        iScore[iTeam]   = 0;
        sText = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam]->setText(sText);
        pTimeoutDecrement[iTeam]->setEnabled(false);
        pTimeoutIncrement[iTeam]->setEnabled(true);
        pSetsDecrement[iTeam]->setEnabled(iSet[iTeam] != 0);
        pSetsIncrement[iTeam]->setEnabled(true);
        pScoreDecrement[iTeam]->setEnabled(false);
        pScoreIncrement[iTeam]->setEnabled(true);
    }
    iServizio   = 0;
    lastService = 0;
    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    sendAll();
    SaveStatus();
}


void
VolleyController::onButtonNewGameClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Iniziare una Nuova Partita ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    prepareScoreFile();
    gsArgs.sTeam[0]    = tr("Locali");
    gsArgs.sTeam[1]    = tr("Ospiti");
    QString sText;
    for(int iTeam=0; iTeam<2; iTeam++) {
        pTeamName[iTeam]->setText(gsArgs.sTeam[iTeam]);
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam]->setText(sText);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        iSet[iTeam]   = 0;
        sText = QString("%1").arg(iSet[iTeam], 1);
        pSetsEdit[iTeam]->setText(sText);
        iScore[iTeam]   = 0;
        sText = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam]->setText(sText);
        pTimeoutDecrement[iTeam]->setEnabled(false);
        pTimeoutIncrement[iTeam]->setEnabled(true);
        pSetsDecrement[iTeam]->setEnabled(false);
        pSetsIncrement[iTeam]->setEnabled(true);
        pScoreDecrement[iTeam]->setEnabled(false);
        pScoreIncrement[iTeam]->setEnabled(true);
    }
    iServizio   = 0;
    lastService = 0;
    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    sendAll();
    pCharts->resetAll();
    pRaceWindow->resetAll();
    SaveStatus();
}


void
VolleyController::onButtonStatisticsClicked() {
    QPixmap* pPixmap = new QPixmap();
    QSize iconSize = QSize(48,48);
    if(pRaceWindow->isVisible()) {
        pRaceWindow->hide();
        pPixmap->load(":/buttonIcons/plot.png");
    }
    else if(pCharts->isVisible()) {
        pCharts->hide();
        pPixmap->load(":/buttonIcons/plot.png");
    }
    else {
        if(setSelectionDialog.exec() == QDialog::Accepted) {
/* TODO: Parte da rimuovere... serve solo per debug. */
            int iScore0 = 0;
            int iScore1 = 0;
            bool bEnd   = false;
            while(!bEnd) {
                if(rand() & 1) iScore0++;
                else iScore1++;
                pCharts->updateScore(iScore0, iScore1, setSelectionDialog.iSelectedSet);
                pRaceWindow->updateScore(iScore0, iScore1, setSelectionDialog.iSelectedSet);
                bEnd = ((iScore0 > 24) || (iScore1 > 24)) &&
                       std::abs(iScore0-iScore1) > 1;
            }
/* Fine parte da rimuovere */
            if(setSelectionDialog.isPlotSelected())
                pCharts->showFullScreen();
            else
                pRaceWindow->showFullScreen();
            if(pRaceWindow->isVisible()) {
                pPixmap->load(":/buttonIcons/sign_stop.png");
                pRaceWindow->startRace(setSelectionDialog.iSelectedSet);
            }
            else if(pCharts->isVisible()) {
                pPixmap->load(":/buttonIcons/sign_stop.png");
                pCharts->startChartAnimation(setSelectionDialog.iSelectedSet);
            }
            else {
                pPixmap->load(":/buttonIcons/plot.png");
            }
        }
        else {
            pPixmap->load(":/buttonIcons/plot.png");
        }
    }
    pStatisticButton->setIcon(QIcon(*pPixmap));
    pStatisticButton->setIconSize(iconSize);
}


void
VolleyController::onStatisticsDone() {
    QPixmap* pPixmap= new QPixmap();
    QSize iconSize = QSize(48,48);
    pPixmap->load(":/buttonIcons/plot.png");
    pStatisticButton->setIcon(QIcon(*pPixmap));
    pStatisticButton->setIconSize(iconSize);
}


void
VolleyController::onRaceDone() {
    pRaceWindow->hide();
    QPixmap* pPixmap= new QPixmap();
    QSize iconSize = QSize(48,48);
    pPixmap->load(":/buttonIcons/plot.png");
    pStatisticButton->setIcon(QIcon(*pPixmap));
    pStatisticButton->setIconSize(iconSize);
}


void
VolleyController::onChangePanelOrientation(PanelOrientation orientation) {
    Q_UNUSED(orientation)
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
               .arg(static_cast<int>(orientation)));
#endif
    pVolleyPanel->setMirrored(gsArgs.isPanelMirrored);
}

