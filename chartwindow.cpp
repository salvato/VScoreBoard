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
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QThread>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QCategoryAxis>

#include "chartwindow.h"
#include "utility.h"


ChartWindow::ChartWindow(QWidget *parent)
    : QWidget{parent}
    , maxX(25)
    , maxY(25)
{
    Q_UNUSED(parent);

    QList<QScreen*> screens = QApplication::screens();
    QRect screenGeometry = screens.at(0)->geometry();
    if(screens.count() > 1) { // Move the Window to the Secondary Display
        screenGeometry = screens.at(1)->geometry();
        QPoint point = QPoint(screenGeometry.x(), screenGeometry.y());
        move(point);
    }

    setWindowIcon(QIcon(":/buttonIcons/plot.png"));

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, screenGeometry.height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);
    panelPalette.setColor(QPalette::WindowText,    Qt::yellow);
    panelPalette.setColor(QPalette::Base,          Qt::black);
    panelPalette.setColor(QPalette::AlternateBase, Qt::blue);
    panelPalette.setColor(QPalette::Text,          Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,    Qt::white);
    setPalette(panelPalette);

    QChart* pChart = createLineChart();
    // The ownership of the chart is passed to pChartView
    pChartView = new QChartView(pChart);
    auto* pLayout = new QGridLayout();
    pLayout->addWidget(pChartView, 0, 0, 1, 1);
    setLayout(pLayout);

    closeTimer.setSingleShot(true);
    connect(&animateTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToAnimate()));
    connect(&closeTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToClose()));
}


QChart*
ChartWindow::createLineChart() {
    QChart* pChart = new QChart();
    pChart->setTheme(QChart::ChartThemeBlueCerulean);
    pChart->setBackgroundVisible(false);
    QFont font = pChart->titleFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pChart->setTitleFont(font);
    pChart->setFont(font);
    pChart->legend()->setFont(font);
    pChart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    QCategoryAxis* pAxisX = new QCategoryAxis();
    pAxisX->setMin(0);
    pAxisX->setMax(maxX);
    font = pAxisX->labelsFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pAxisX->setLabelsFont(font);
    pAxisX->setLabelFormat("%d");

    QValueAxis* pAxisY = new QCategoryAxis();
    pAxisY->setMin(0);
    pAxisY->setMax(maxY);
    font = pAxisY->labelsFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pAxisY->setLabelsFont(font);
    // Add a "blank" to the label to add space
    // between labels and the vertical axis
    pAxisY->setLabelFormat("%d  ");

    pChart->addAxis(pAxisX, Qt::AlignBottom);
    pChart->addAxis(pAxisY, Qt::AlignLeft);

    QLineSeries* pScoreSequence;
    for(int i=0; i<2; i++) {
        pScoreSequence = new QLineSeries();
        pScoreSequence->append(0, 0);
        pChart->addSeries(pScoreSequence);
        pScoreSequence->attachAxis(pAxisX);
        pScoreSequence->attachAxis(pAxisY);
        font = pScoreSequence->pointLabelsFont();
        font.setPointSize(32);
        pScoreSequence->setPointLabelsFont(font);
        QPen pen = pScoreSequence->pen();
        i==0 ? pen.setColor(Qt::yellow) : pen.setColor(Qt::red);
        pen.setWidth(pen.width()*5);
        pScoreSequence->setPen(pen);
    }
    return pChart;
}


void
ChartWindow::updateLabel(int iTeam, QString sLabel) {
    if((iTeam < 0) || (iTeam > 1)) return;
    sTeamName[iTeam] = sLabel;
    update();
}


void
ChartWindow::updateScore(int team0Score, int team1Score, int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    int tScore = team0Score+team1Score;
    if(!score[iSet].isEmpty()) {
        while(tScore <= score[iSet].last().x()+score[iSet].last().y()) {
            score[iSet].removeLast();
            if(score[iSet].isEmpty()) {
                score[iSet].append(QVector2D(0, 0));
                break;
            }
        }
    }
    score[iSet].append(QVector2D(team0Score, team1Score));
    maxScore[iSet] = std::max(team0Score, team1Score);
}


void
ChartWindow::resetScore(int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    score[iSet].clear();
    score[iSet].append(QVector2D(0, 0));
    maxScore[iSet] = 0;
}


void
ChartWindow::resetAll() {
    for(int i=0; i<5; i++) {
        score[i].clear();
        score[i].append(QVector2D(0, 0));
        maxScore[i] = 0;
    }
}


void
ChartWindow::startChartAnimation(int iSet) {
    iCurrentSet = iSet;
    indexScore = 0;
    int maxX = score[iSet].last().x()+score[iSet].last().y();
    QChart* pChart = pChartView->chart();
    pChart->setTitle(QString("Set %1").arg(iSet+1));

    QCategoryAxis* pAxisX;
    pAxisX = reinterpret_cast<QCategoryAxis*>(pChart->axes(Qt::Horizontal).constFirst());
    QStringList categoriesLabels = pAxisX->categoriesLabels();
    for(int i=0; i<categoriesLabels.count(); i++)
        pAxisX->remove(categoriesLabels.at(i));
    pAxisX->setMin(0);
    pAxisX->setMax(maxX);
    pAxisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    for(int i=0; i<=maxX; i+=5) {
        pAxisX->append(QString("%1").arg(i), i);
    }
    if(maxX % 5)
        pAxisX->append(QString("%1").arg(maxX), maxX);

    QCategoryAxis* pAxisY;
    pAxisY = reinterpret_cast<QCategoryAxis*>(pChart->axes(Qt::Vertical).constFirst());
    categoriesLabels = pAxisY->categoriesLabels();
    for(int i=0; i<categoriesLabels.count(); i++)
        pAxisY->remove(categoriesLabels.at(i));
    pAxisY->setMin(0);
    pAxisY->setMax(maxScore[iCurrentSet]);
    pAxisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    for(int i=0; i<=maxScore[iCurrentSet]; i+=5) {
        pAxisY->append(QString("%1").arg(i), i);
    }
    if(maxScore[iCurrentSet] % 5)
        pAxisY->append(QString("%1").arg(maxScore[iCurrentSet]), maxScore[iCurrentSet]);

    QLineSeries* pScoreSequence;
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->clear();
    pScoreSequence->append(0,0);
    pScoreSequence->setName(sTeamName[0]);
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->clear();
    pScoreSequence->append(0,0);
    pScoreSequence->setName(sTeamName[1]);
    animateTimer.start(1000);
}


void
ChartWindow::hide() {
    animateTimer.stop();
    closeTimer.stop();
    QWidget::hide();
}


void
ChartWindow::onTimeToAnimate() {
    QChart* pChart = pChartView->chart();
    int x = score[iCurrentSet].at(indexScore).x()+score[iCurrentSet].at(indexScore).y();
    QLineSeries* pScoreSequence;
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->append(x, score[iCurrentSet].at(indexScore).x());
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->append(x, score[iCurrentSet].at(indexScore).y());
    indexScore++;
    if(indexScore >= score[iCurrentSet].count()) {
        animateTimer.stop();
        closeTimer.start(5000);
    }
    update();
}


void
ChartWindow::onTimeToClose() {
    emit done();
    animateTimer.stop();
    closeTimer.stop();
    close();
}
