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
#pragma once

#include <QMainWindow>
#include <QFile>
#include <QSettings>


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
