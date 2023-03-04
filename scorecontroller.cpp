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
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QScreen>


#include "scorecontroller.h"


ScoreController::ScoreController(QFile *myLogFile, QWidget *parent)
    : QMainWindow(parent)
    , pLogFile(myLogFile)
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

    setWindowTitle("Score Controller");

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
    pGeneralSetupButton = new QPushButton(ButtonIcon, "");
    pGeneralSetupButton->setIconSize(pixmap.rect().size());
    pGeneralSetupButton->setFlat(true);
    pGeneralSetupButton->setToolTip("General Setup");

    pixmap.load(":/buttonIcons/video-display.png");
    ButtonIcon.addPixmap(pixmap);
    pShutdownButton = new QPushButton(ButtonIcon, "");
    pShutdownButton->setIconSize(pixmap.rect().size());
    pShutdownButton->setFlat(true);
    pShutdownButton->setToolTip("Shutdown System");

    spotButtonLayout->addWidget(pSpotButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pSlideShowButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pGeneralSetupButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pShutdownButton);

    return spotButtonLayout;
}


void
ScoreController::connectButtonSignals() {
    connect(pSpotButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSpotLoopClicked()));

    connect(pSlideShowButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSlideShowClicked()));
    connect(pGeneralSetupButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSetupClicked()));

    connect(pShutdownButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonShutdownClicked()));
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


// Dummy... see Volley Panel
void
ScoreController::startSpotLoop() {
}


// Dummy... see Volley Panel
void
ScoreController::stopSpotLoop() {
}


// Dummy... see Volley Panel
void
ScoreController::startSlideShow() {
}


// Dummy... see Volley Panel
void
ScoreController::stopSlideShow() {
}


void
ScoreController::UpdateUI() {
    pSpotButton->setEnabled(true);
    pSlideShowButton->setEnabled(true);
    pShutdownButton->setEnabled(true);
}


void
ScoreController::onButtonSpotLoopClicked() {
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        startSpotLoop();
        myStatus = showSpots;
    }
    else {
        pixmap.load(":/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        stopSpotLoop();
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonSlideShowClicked() {
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        pSpotButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        startSlideShow();
        myStatus = showSlides;
    }
    else {
        pSpotButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
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

