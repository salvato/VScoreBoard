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

    // The default Directories to look for the slides and spots
    sSlideDir   = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
    sSpotDir    = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");

    slideList     = QFileInfoList();
    spotList      = QFileInfoList();
    iCurrentSlide = 0;
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
    startStopLoopSpotButton = new QPushButton(ButtonIcon, "");
    startStopLoopSpotButton->setIconSize(pixmap.rect().size());
    startStopLoopSpotButton->setFlat(true);
    startStopLoopSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/buttonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    startStopSlideShowButton = new QPushButton(ButtonIcon, "");
    startStopSlideShowButton->setIconSize(pixmap.rect().size());
    startStopSlideShowButton->setFlat(true);
    startStopSlideShowButton->setToolTip("Start/Stop Slide Show");

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

    spotButtonLayout->addWidget(startStopLoopSpotButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(startStopSlideShowButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(generalSetupButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(shutdownButton);

    return spotButtonLayout;
}


void
ScoreController::connectButtonSignals() {
        connect(startStopLoopSpotButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSpotLoopClicked()));
        connect(startStopLoopSpotButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(startStopSlideShowButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSlideShowClicked()));
        connect(startStopSlideShowButton, SIGNAL(clicked()),
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
ScoreController::prepareDirectories() {
    QDir slideDir(sSlideDir);
    QDir spotDir(sSpotDir);

    if(!slideDir.exists() || !spotDir.exists()) {
        onButtonSetupClicked();
        slideDir.setPath(sSlideDir);
        if(!slideDir.exists())
            sSlideDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
        if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
        spotDir.setPath(sSpotDir);
        if(!spotDir.exists())
            sSpotDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
        if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");
        pSettings->setValue("directories/slides", sSlideDir);
        pSettings->setValue("directories/spots", sSpotDir);
    }
    else {
        QStringList filter(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.JPG" << "*.JPEG" << "*.PNG");
        slideDir.setNameFilters(filter);
        slideList = slideDir.entryInfoList();
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Slides directory: %1 Found %2 Slides")
                   .arg(sSlideDir)
                   .arg(slideList.count()));
#endif
        QStringList nameFilter(QStringList() << "*.mp4"<< "*.MP4");
        spotDir.setNameFilters(nameFilter);
        spotDir.setFilter(QDir::Files);
        spotList = spotDir.entryInfoList();
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Spot directory: %1 Found %2 Spots")
                   .arg(sSpotDir)
                   .arg(spotList.count()));
#endif
    }
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
    startStopLoopSpotButton->setEnabled(true);
    startStopSlideShowButton->setEnabled(true);
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
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());
        startStopSlideShowButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        startSpotLoop();
        myStatus = showSpots;
    }
    else {
        pixmap.load(":/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());
        startStopSlideShowButton->setEnabled(true);
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
        startStopLoopSpotButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());
        startSlideShow();
        myStatus = showSlides;
    }
    else {
        startStopLoopSpotButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        pixmap.load(":/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());
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

