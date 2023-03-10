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

#include <QtGlobal>
#include <QtWidgets>
#include <QProcess>
#include <QVBoxLayout>
#include <QDebug>


#include "slidewidget.h"
#include "scorepanel.h"
#include "utility.h"


ScorePanel::ScorePanel(QFile *myLogFile, QWidget *parent)
    : QWidget(parent)
    , isMirrored(false)
    , pLogFile(myLogFile)
    , pVideoPlayer(nullptr)
    , iCurrentSpot(0)
    , pMySlideWindow(new SlideWidget())
    , pPanel(nullptr)
#ifdef Q_OS_WINDOWS
    , sVideoPlayer(QString("ffplay.exe"))
#else
    , sVideoPlayer(QString("/usr/bin/ffplay"))
#endif

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
    QPoint point = QPoint(screens.at(1)->geometry().x(),
                          screens.at(1)->geometry().y());
    move(point);

    setWindowTitle("Score Panel");

    // We want the cursor set for all widgets,
    // even when outside the window then:
    setCursor(Qt::BlankCursor);
    // We don't want windows decorations
    setWindowFlags(Qt::CustomizeWindowHint);

    QString sBaseDir;
    sBaseDir = QDir::homePath();
    if(!sBaseDir.endsWith(QString("/"))) sBaseDir+= QString("/");

    sSpotDir = QString("%1spots/").arg(sBaseDir);
    sSlideDir= QString("%1slides/").arg(sBaseDir);

}


ScorePanel::~ScorePanel() {
    doProcessCleanup();
}


void
ScorePanel::buildLayout() {
    QWidget* oldPanel = pPanel;
    pPanel = new QWidget(this);
    QVBoxLayout* pPanelLayout = new QVBoxLayout();
    pPanelLayout->addLayout(createPanel());
    pPanel->setLayout(pPanelLayout);
    if(!layout()) {
        QVBoxLayout* pMainLayout = new QVBoxLayout();
        setLayout(pMainLayout);
     }
    layout()->addWidget(pPanel);
    if(oldPanel != nullptr)
        delete oldPanel;
}


//==================
// Panel management
//==================
void
ScorePanel::doProcessCleanup() {
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Cleaning all processes"));
#endif

    if(pMySlideWindow) {
        pMySlideWindow->close();
    }
    if(pVideoPlayer) {
        pVideoPlayer->disconnect();
        pVideoPlayer->close();
#ifdef LOG_MESG
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Closing Video Player..."));
#endif
        pVideoPlayer->waitForFinished(3000);
        pVideoPlayer->deleteLater();
        pVideoPlayer = Q_NULLPTR;
    }
}


void
ScorePanel::closeEvent(QCloseEvent *event) {
    doProcessCleanup();
    event->accept();
}


void
ScorePanel::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        close();
    }
}


void
ScorePanel::setSlideDir(QString sDir) {
    sSlideDir = sDir;
}


void
ScorePanel::setSpotDir(QString sDir) {
    sSpotDir = sDir;
}


void
ScorePanel::onSpotClosed(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    if(pVideoPlayer) {
        pVideoPlayer->disconnect();
        pVideoPlayer->close();// Closes all communication with the process and kills it.
        delete pVideoPlayer;
        pVideoPlayer = nullptr;
    } // if(pVideoPlayer)
    showFullScreen(); // Restore the Score Panel
}


void
ScorePanel::onStartNextSpot(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    showFullScreen(); // Ripristina lo Score Panel
    // Update spot list just in case we are updating the spot list...
    QDir spotDir(sSpotDir);
    spotList = QFileInfoList();
    QStringList nameFilter(QStringList() << "*.mp4" << "*.MP4");
    spotDir.setNameFilters(nameFilter);
    spotDir.setFilter(QDir::Files);
    spotList = spotDir.entryInfoList();
    if(spotList.count() == 0) {
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("No spots available !"));
#endif
        if(pVideoPlayer) {
            pVideoPlayer->disconnect();
            delete pVideoPlayer;
            pVideoPlayer = nullptr;
        }
        return;
    }

    iCurrentSpot = iCurrentSpot % spotList.count();
    if(!pVideoPlayer) {
        pVideoPlayer = new QProcess(this);
        connect(pVideoPlayer, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(onStartNextSpot(int,QProcess::ExitStatus)));
    }

    QStringList sArguments;
    sArguments = QStringList{"-noborder",
                             "-sn",
                             "-autoexit",
                             "-fs"
                            };
    QList<QScreen*> screens = QApplication::screens();
    if(screens.count() > 1) {
        QRect screenres = screens.at(1)->geometry();
        sArguments.append(QString("-left"));
        sArguments.append(QString("%1").arg(screenres.x()));
        sArguments.append(QString("-top"));
        sArguments.append(QString("%1").arg(screenres.y()));
        sArguments.append(QString("-x"));
        sArguments.append(QString("%1").arg(screenres.width()));
        sArguments.append(QString("-y"));
        sArguments.append(QString("%1").arg(screenres.height()));
    }
    sArguments.append(spotList.at(iCurrentSpot).absoluteFilePath());

    pVideoPlayer->start(sVideoPlayer, sArguments);
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Now playing: %1")
               .arg(spotList.at(iCurrentSpot).absoluteFilePath()));
#endif
    iCurrentSpot = (iCurrentSpot+1) % spotList.count();// Prepare Next Spot
    if(!pVideoPlayer->waitForStarted(3000)) {
        pVideoPlayer->close();
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Impossibile mandare lo spot"));
        pVideoPlayer->disconnect();
        delete pVideoPlayer;
        pVideoPlayer = Q_NULLPTR;
        return;
    }
    hide();
}


void
ScorePanel::startSpotLoop() {
    QDir spotDir(sSpotDir);
    spotList = QFileInfoList();
    if(spotDir.exists()) {
        QStringList nameFilter(QStringList() << "*.mp4" << "*.MP4");
        spotDir.setNameFilters(nameFilter);
        spotDir.setFilter(QDir::Files);
        spotList = spotDir.entryInfoList();
    }
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Found %1 spots").arg(spotList.count()));
#endif
    if(!spotList.isEmpty()) {
        iCurrentSpot = iCurrentSpot % spotList.count();
        if(!pVideoPlayer) {
            pVideoPlayer = new QProcess(this);
            connect(pVideoPlayer, SIGNAL(finished(int,QProcess::ExitStatus)),
                    this, SLOT(onStartNextSpot(int,QProcess::ExitStatus)));

            QStringList sArguments;
            sArguments = QStringList{"-noborder",
                                     "-sn",
                                     "-autoexit",
                                     "-fs"
                                    };
            QList<QScreen*> screens = QApplication::screens();
            if(screens.count() > 1) {
                QRect screenres = screens.at(1)->geometry();
                sArguments.append(QString("-left"));
                sArguments.append(QString("%1").arg(screenres.x()));
                sArguments.append(QString("-top"));
                sArguments.append(QString("%1").arg(screenres.y()));
                sArguments.append(QString("-x"));
                sArguments.append(QString("%1").arg(screenres.width()));
                sArguments.append(QString("-y"));
                sArguments.append(QString("%1").arg(screenres.height()));
            }
            sArguments.append(spotList.at(iCurrentSpot).absoluteFilePath());

            pVideoPlayer->start(sVideoPlayer, sArguments);
#ifdef LOG_VERBOSE
            logMessage(pLogFile,
                       Q_FUNC_INFO,
                       QString("Now playing: %1")
                       .arg(spotList.at(iCurrentSpot).absoluteFilePath()));
#endif
            iCurrentSpot = (iCurrentSpot+1) % spotList.count();// Prepare Next Spot
            if(!pVideoPlayer->waitForStarted(3000)) {
                pVideoPlayer->close();
                logMessage(pLogFile,
                           Q_FUNC_INFO,
                           QString("Impossibile mandare lo spot."));
                pVideoPlayer->disconnect();
                delete pVideoPlayer;
                pVideoPlayer = nullptr;
                return;
            }
            hide(); // Hide the Score Panel
        } // if(!videoPlayer)
    }
}


void
ScorePanel::stopSpotLoop() {
    if(pVideoPlayer) {
        pVideoPlayer->disconnect();
        connect(pVideoPlayer, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(onSpotClosed(int,QProcess::ExitStatus)));
        pVideoPlayer->terminate();
    }
}


void
ScorePanel::startSlideShow() {
    if(pVideoPlayer)// || pCameraPlayer)
        return;// No Slide Show if movies are playing or camera is active
    if(pMySlideWindow) {
        pMySlideWindow->setSlideDir(sSlideDir);
        pMySlideWindow->showFullScreen();
        hide(); // Hide the Score Panel
        pMySlideWindow->startSlideShow();
    }
    else {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Invalid Slide Window"));
    }
}


void
ScorePanel::stopSlideShow() {
    if(pMySlideWindow) {
        pMySlideWindow->stopSlideShow();
        showFullScreen(); // Show the Score Panel
        pMySlideWindow->hide();
    }
}


QGridLayout*
ScorePanel::createPanel() {
    return new QGridLayout();
}
