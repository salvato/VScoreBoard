#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QThread>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QScreen>


#include "scorecontroller.h"
#include "utility.h"


ScoreController::ScoreController(QFile *myLogFile, QWidget *parent)
    : QMainWindow(parent)
    , pLogFile(myLogFile)
    , pButtonClick(nullptr)
    , pSettings(new QSettings("Gabriele Salvato", "Volley Controller"))
{
    QList<QScreen*> screens = QApplication::screens();
    if(screens.count() < 2) {
        QMessageBox::critical(nullptr,
                              QObject::tr("Secondo Monitor non connesso"),
                              QObject::tr("Connettilo e Riesegui il Programma"),
                              QMessageBox::Abort);
        exit(EXIT_FAILURE);
    }
    // Move the Panel on the Secondary Display
    QPoint point = QPoint(screens.at(0)->geometry().x(),
                          screens.at(0)->geometry().y());
    move(point);
    setWindowTitle("Score Controller");

    // The click sound for button press.
    pButtonClick = new QSoundEffect(this);
    pButtonClick->setSource(QUrl::fromLocalFile(":/key.wav"));

    iCurrentSpot  = 0;

    pSpotButtonsLayout = CreateSpotButtons();
    connectButtonSignals();

    myStatus = showPanel;
}


ScoreController::~ScoreController() {
}


QHBoxLayout*
ScoreController::CreateSpotButtons() {
    auto* spotButtonLayout = new QHBoxLayout();

    QPixmap pixmap(":/buttonIcons/PlaySpots.png");
    QIcon ButtonIcon(pixmap);
    pSpotButton = new QPushButton(ButtonIcon, "");
    pSpotButton->setIconSize(pixmap.rect().size());
    pSpotButton->setFlat(true);
    pSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/buttonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    pSlideShowButton = new QPushButton(ButtonIcon, "");
    pSlideShowButton->setIconSize(pixmap.rect().size());
    pSlideShowButton->setFlat(true);
    pSlideShowButton->setToolTip("Start/Stop Slide Show");

    pixmap.load(":/buttonIcons/PanelSetup.png");
    ButtonIcon.addPixmap(pixmap);
    generalSetupButton = new QPushButton(ButtonIcon, "");
    generalSetupButton->setIconSize(pixmap.rect().size());
    generalSetupButton->setFlat(true);
    generalSetupButton->setToolTip("General Setup");

    pixmap.load(":/buttonIcons/video-display.png");
    ButtonIcon.addPixmap(pixmap);
    shutdownButton = new QPushButton(ButtonIcon, "");
    shutdownButton->setIconSize(pixmap.rect().size());
    shutdownButton->setFlat(true);
    shutdownButton->setToolTip("Shutdown System");

    spotButtonLayout->addWidget(pSpotButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pSlideShowButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(generalSetupButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(shutdownButton);

    return spotButtonLayout;
}


void
ScoreController::connectButtonSignals() {
        connect(pSpotButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSpotLoopClicked()));
        connect(pSpotButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(pSlideShowButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSlideShowClicked()));
        connect(pSlideShowButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(generalSetupButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonSetupClicked()));
        connect(generalSetupButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(shutdownButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonShutdownClicked()));
        connect(shutdownButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
}


void
ScoreController::onButtonSetupClicked() {
    GeneralSetup();
}


// Dummy... see Volley Panel
void
ScoreController::GeneralSetup() {
}


// Dummy... see Volley Panel
void
ScoreController::SaveStatus() {
}


void
ScoreController::UpdateUI() {
    pSpotButton->setEnabled(true);
    pSlideShowButton->setEnabled(true);
    shutdownButton->setEnabled(true);
}


void
ScoreController::startSpotLoop() {
}


void
ScoreController::stopSpotLoop() {
}


void
ScoreController::startSlideShow() {
}


void
ScoreController::stopSlideShow() {
}


void
ScoreController::onButtonStartStopSpotLoopClicked() {
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        startSpotLoop();
        myStatus = showSpots;
    }
    else {
        pixmap.load(":/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        stopSpotLoop();
        myStatus = showPanel;
    }
}


void
ScoreController::onGetPanelDirection() {
}


void
ScoreController::onSetPanelDirection(PanelOrientation direction) {
    Q_UNUSED(direction)
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
               .arg(static_cast<int>(direction)));
#endif
}


void
ScoreController::onButtonStartStopSlideShowClicked() {
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        pSpotButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        startSlideShow();
        myStatus = showSlides;
    }
    else {
        pSpotButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        pixmap.load(":/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        stopSlideShow();
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonShutdownClicked() {
    QMessageBox msgBox;
    msgBox.setText("Sei Sicuro di Volere Spegnere");
    msgBox.setInformativeText("i Tabelloni ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int answer = msgBox.exec();
    if(answer != QMessageBox::Yes) return;
    close();
}

