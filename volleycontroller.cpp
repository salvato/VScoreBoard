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


VolleyController::VolleyController(QFile *myLogFile, QWidget *parent)
    : ScoreController(myLogFile, parent)
    , pVolleyPanel(new VolleyPanel(myLogFile))
    , isPanelMirrored(true)
    , bFontBuilt(false)
{
    setWindowIcon(QIcon(":/Logo.ico"));

    // QWidget propagates explicit palette roles from parent to child.
    // If you assign a brush or color to a specific role on a palette and
    // assign that palette to a widget, that role will propagate to all
    // the widget's children, overriding any system defaults for that role.
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
//    panelPalette.setColor(QPalette::HighlightedText, Qt::white);
//    panelPalette.setColor(QPalette::Highlight,       QColor(0, 0, 0, 0));

    setPalette(panelPalette);

    GetSettings();

    prepareDirectories();

    buildControls();
    setWindowLayout();

    setEventHandlers();

    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pService[iServizio ? 0 : 1]->setFocus();

    pVolleyPanel->showFullScreen();
}


void
VolleyController::closeEvent(QCloseEvent *event) {
    SaveSettings();
    delete pVolleyPanel;
    event->accept();
}


void
VolleyController::resizeEvent(QResizeEvent *event) {
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


void
VolleyController::GeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&generalSetupArguments);
    if(isPanelMirrored)
        pGeneralSetupDialog->setCurrrentOrientaton(PanelOrientation::Reflected);
    else
        pGeneralSetupDialog->setCurrrentOrientaton(PanelOrientation::Reflected);
    connect(pGeneralSetupDialog, SIGNAL(changeOrientation(PanelOrientation)),
            this, SLOT(onChangePanelOrientation(PanelOrientation)));
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        sSlideDir = generalSetupArguments.sSlideDir;
        if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
        QDir slideDir(sSlideDir);
        if(sSlideDir != QString() && slideDir.exists()) {
            QStringList filter(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.JPG" << "*.JPEG" << "*.PNG");
            slideDir.setNameFilters(filter);
            slideList = slideDir.entryInfoList();
        }
        else {
            sSlideDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
            slideList = QFileInfoList();
        }
#ifdef LOG_MESSAGE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Found %1 slides").arg(slideList.count()));
#endif
        sSpotDir = generalSetupArguments.sSpotDir;
        if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");
        QDir spotDir(sSpotDir);
        if(sSpotDir != QString() && spotDir.exists()) {
            QStringList nameFilter(QStringList() << "*.mp4" << "*.MP4");
            spotDir.setNameFilters(nameFilter);
            spotDir.setFilter(QDir::Files);
            spotList = spotDir.entryInfoList();
        }
        else {
            sSpotDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
            spotList = QFileInfoList();
        }
#ifdef LOG_MESSAGE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Found %1 spots")
                   .arg(spotList.count()));
#endif
        SaveSettings();
    }
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

    QPixmap pixmap(":/buttonIcons/ExchangeVolleyField.png");
    QIcon ButtonIcon(pixmap);
    changeFieldButton = new QPushButton(ButtonIcon, "");
    changeFieldButton->setIconSize(pixmap.rect().size());
    changeFieldButton->setFlat(true);
    changeFieldButton->setToolTip("Inverti Campo");

    pixmap.load(":/buttonIcons/New-Game-Volley.png");
    ButtonIcon.addPixmap(pixmap);
    pNewGameButton = new QPushButton(ButtonIcon, "");
    pNewGameButton->setIconSize(pixmap.rect().size());
    pNewGameButton->setFlat(true);
    pNewGameButton->setToolTip("Nuova Partita");

    pixmap.load(":/buttonIcons/New-Set-Volley.png");
    ButtonIcon.addPixmap(pixmap);
    pNewSetButton  = new QPushButton(ButtonIcon, "");
    pNewSetButton->setIconSize(pixmap.rect().size());
    pNewSetButton->setFlat(true);
    pNewSetButton->setToolTip("Nuovo Set");

    gameButtonLayout->addWidget(pNewGameButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pNewSetButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(changeFieldButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
VolleyController::GetSettings() {
    generalSetupArguments.maxTimeout       = pSettings->value("volley/maxTimeout", 2).toInt();
    generalSetupArguments.maxSet           = pSettings->value("volley/maxSet", 3).toInt();
    generalSetupArguments.iTimeoutDuration = pSettings->value("volley/TimeoutDuration", 30).toInt();
    generalSetupArguments.sSlideDir        = pSettings->value("directories/slides", sSlideDir).toString();
    generalSetupArguments.sSpotDir         = pSettings->value("directories/spots",  sSpotDir).toString();

    sTeam[0]    = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    sTeam[1]    = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();
    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iSet[0]     = pSettings->value("team1/sets", 0).toInt();
    iSet[1]     = pSettings->value("team2/sets", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iServizio   = pSettings->value("set/service", 0).toInt();
    lastService = pSettings->value("set/lastservice", 0).toInt();

    sSlideDir   = generalSetupArguments.sSlideDir;
    sSpotDir    = generalSetupArguments.sSpotDir;

    isPanelMirrored  = pSettings->value("panel/orientation",  true).toBool();

    // Check Stored Values vs Maximum Values
    for(int i=0; i<2; i++) {
        if(iTimeout[i] > generalSetupArguments.maxTimeout)
            iTimeout[i] = generalSetupArguments.maxTimeout;
        if(iSet[i] > generalSetupArguments.maxSet)
            iSet[i] = generalSetupArguments.maxSet;
    }

    sendAll();
}


void
VolleyController::sendAll() {
    pVolleyPanel->setTeam(0, sTeam[0]);
    pVolleyPanel->setTeam(1, sTeam[1]);
    pVolleyPanel->setTimeout(0, iTimeout[0]);
    pVolleyPanel->setTimeout(1, iTimeout[1]);
    pVolleyPanel->setSets(0, iSet[0]);
    pVolleyPanel->setSets(1, iSet[1]);
    pVolleyPanel->setScore(0, iScore[0]);
    pVolleyPanel->setScore(1, iScore[1]);
    pVolleyPanel->setServizio(iServizio);
    pVolleyPanel->setMirrored(isPanelMirrored);
    // Le Directories ???
}


void
VolleyController::SaveStatus() {
    // Save Present Game Values
    pSettings->setValue("team1/name", sTeam[0]);
    pSettings->setValue("team2/name", sTeam[1]);
    pSettings->setValue("team1/timeouts", iTimeout[0]);
    pSettings->setValue("team2/timeouts", iTimeout[1]);
    pSettings->setValue("team1/sets", iSet[0]);
    pSettings->setValue("team2/sets", iSet[1]);
    pSettings->setValue("team1/score", iScore[0]);
    pSettings->setValue("team2/score", iScore[1]);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
    pSettings->setValue("panel/orientation", pVolleyPanel->getMirrored());
}


void
VolleyController::SaveSettings() { // Save General Setup Values
    pSettings->setValue("directories/slides",     generalSetupArguments.sSlideDir);
    pSettings->setValue("directories/spots",      generalSetupArguments.sSpotDir);
    pSettings->setValue("volley/maxTimeout",      generalSetupArguments.maxTimeout);
    pSettings->setValue("volley/maxSet",          generalSetupArguments.maxSet);
    pSettings->setValue("volley/TimeoutDuration", generalSetupArguments.iTimeoutDuration);
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
        pTeamName[iTeam] = new Edit(sTeam[iTeam], iTeam);
        pTeamName[iTeam]->setAlignment(Qt::AlignHCenter);
        pTeamName[iTeam]->setMaxLength(15);
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
        if(iTimeout[iTeam] == generalSetupArguments.maxTimeout) {
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
        if(iSet[iTeam] == generalSetupArguments.maxSet)
            pSetsIncrement[iTeam]->setEnabled(false);
        // Service
        QPixmap pixmap(":/buttonIcons/ball.png");
        QIcon ButtonIcon(pixmap);
        pService[iTeam] = new Button("", iTeam);
        pService[iTeam]->setIcon(ButtonIcon);
        auto const rec = QApplication::primaryScreen()->availableSize();
        auto const height = rec.height();
        pService[iTeam]->setIconSize(QSize(height/16,height/16));
        pService[iTeam]->setCheckable(true);
        pService[iTeam]->setStyleSheet("QPushButton:checked { background-color: rgb(0, 0, 196) }");
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
        connect(pTimeoutIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pTimeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(pTimeoutDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pSetsIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetIncrement(int)));
        connect(pSetsIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pSetsDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetDecrement(int)));
        connect(pSetsDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pService[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onServiceClicked(int)));
        connect(pService[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pScoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(pScoreIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(pScoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
        connect(pScoreDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
    }
    // New Set
    connect(pNewSetButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewSetClicked()));
    connect(pNewSetButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    // New Game
    connect(pNewGameButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewGameClicked()));
    connect(pNewGameButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    // Exchange Field Position
    connect(changeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
    connect(changeFieldButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
}


void
VolleyController::startSpotLoop() {
    pVolleyPanel->setSpotDir(sSpotDir);
    pVolleyPanel->startSpotLoop();
}


void
VolleyController::stopSpotLoop() {
    pVolleyPanel->stopSpotLoop();
}


void
VolleyController::startSlideShow() {
    pVolleyPanel->setSlideDir(sSlideDir);
    pVolleyPanel->startSlideShow();
}


void
VolleyController::stopSlideShow() {
    pVolleyPanel->stopSlideShow();
}


// =========================
// Event management routines
// =========================

void
VolleyController::onTimeOutIncrement(int iTeam) {
    iTimeout[iTeam]++;
    if(iTimeout[iTeam] >= generalSetupArguments.maxTimeout) {
        pTimeoutIncrement[iTeam]->setEnabled(false);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:red; border: none");
    }
    pTimeoutDecrement[iTeam]->setEnabled(true);
    pVolleyPanel->setTimeout(iTeam, iTimeout[iTeam]);
    if(generalSetupArguments.iTimeoutDuration > 0) {
        pVolleyPanel->startTimeout(generalSetupArguments.iTimeoutDuration);
    }
    QString sText = QString("%1").arg(iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
    pTimeoutEdit[iTeam]->setFocus();
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
    if(iSet[iTeam] == generalSetupArguments.maxSet) {
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
}


void
VolleyController::onTeamTextChanged(QString sText, int iTeam) {
    sTeam[iTeam] = sText;
    pVolleyPanel->setTeam(iTeam, sTeam[iTeam]);
    sText = QString("team%1/name").arg(iTeam+1, 1);
    pSettings->setValue(sText, sTeam[iTeam]);
}


void
VolleyController::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = sTeam[0];
    sTeam[0] = sTeam[1];
    sTeam[1] = sText;
    pTeamName[0]->setText(sTeam[0]);
    pTeamName[1]->setText(sTeam[1]);

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
        if(iSet[iTeam] == generalSetupArguments.maxSet) {
            pSetsIncrement[iTeam]->setEnabled(false);
        }

        pTimeoutIncrement[iTeam]->setEnabled(true);
        pTimeoutDecrement[iTeam]->setEnabled(true);
        pTimeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow; border: none");
        if(iTimeout[iTeam] == generalSetupArguments.maxTimeout) {
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
    QString sText = sTeam[0];
    sTeam[0] = sTeam[1];
    sTeam[1] = sText;
    pTeamName[0]->setText(sTeam[0]);
    pTeamName[1]->setText(sTeam[1]);
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
    sTeam[0]    = tr("Locali");
    sTeam[1]    = tr("Ospiti");
    QString sText;
    for(int iTeam=0; iTeam<2; iTeam++) {
        pTeamName[iTeam]->setText(sTeam[iTeam]);
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
    SaveStatus();
}


void
VolleyController::onChangePanelOrientation(PanelOrientation orientation) {
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
               .arg(static_cast<int>(orientation)));
#endif
    isPanelMirrored = orientation == PanelOrientation::Reflected;
    pVolleyPanel->setMirrored(isPanelMirrored);
}

