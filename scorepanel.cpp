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


#include "scorepanel.h"
#include "utility.h"


ScorePanel::ScorePanel(QFile *myLogFile, QWidget *parent)
    : QWidget(parent)
    , isMirrored(false)
    , pLogFile(myLogFile)
    , pPanel(nullptr)
{
    (void)START_GRADIENT; // Just to Silent a Warning
// TODO: Remove
//    QList<QScreen*> screens = QApplication::screens();
//    if(screens.count() < 2) {
//        QMessageBox::critical(nullptr,
//                              QObject::tr("Secondo Monitor non connesso"),
//                              QObject::tr("Connettilo e Riesegui il Programma"),
//                              QMessageBox::Abort);
//        exit(EXIT_FAILURE);
//    }
//    // Move the Panel on the Secondary Display
//    QPoint point = QPoint(screens.at(1)->geometry().x(),
//                          screens.at(1)->geometry().y());
//    move(point);

    setWindowTitle("Score Panel");

    // We want the cursor set for all widgets,
    // even when outside the window then:
    setCursor(Qt::BlankCursor);
    // We don't want windows decorations
    setWindowFlags(Qt::CustomizeWindowHint);
}


ScorePanel::~ScorePanel() {
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
ScorePanel::closeEvent(QCloseEvent *event) {
    event->accept();
}


void
ScorePanel::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        close();
    }
}


QGridLayout*
ScorePanel::createPanel() {
    return new QGridLayout();
}
