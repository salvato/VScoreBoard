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
#include <QGridLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "chartwindow.h"
#include "utility.h"


ChartWindow::ChartWindow(QWidget *parent)
    : QWidget{parent}
    , maxScore(25)
{
    Q_UNUSED(parent);

    QList<QScreen*> screens = QApplication::screens();
    QRect screenGeometry = screens.at(0)->geometry();
    if(screens.count() > 1) {
        // Move the Window on the Secondary Display
        screenGeometry = screens.at(1)->geometry();
        QPoint point = QPoint(screenGeometry.x(),
                              screenGeometry.y());
        move(point);
    }

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

    auto* pLayout = new QGridLayout();
    for(int i=0; i<5; i++) {
        QChart* pChart = createLineChart();
        pChart->setTitle(QString("Set %1").arg(i+1));
        chartVector.append(pChart);
        QChartView* pChartView = new QChartView(pChart);
        pLayout->addWidget(pChartView, i/2, i%2, 1, 1);
    }
    setLayout(pLayout);
}


QChart*
ChartWindow::createLineChart()
{
    QChart* pChart = new QChart();

    QLineSeries* pScoreSequence;
    pScoreSequence = new QLineSeries();
    pScoreSequence->append(QPointF(0.0, 0.0));
    pChart->addSeries(pScoreSequence);
    pScoreSequence = new QLineSeries();
    pScoreSequence->append(QPointF(0.0, 0.0));
    pChart->addSeries(pScoreSequence);

    pChart->createDefaultAxes();
    pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxScore);
    pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxScore);

    // Add space to label to add space between labels and vertical axis
    QValueAxis* pAxisY = qobject_cast<QValueAxis*>(pChart->axes(Qt::Vertical).constFirst());
    Q_ASSERT(pAxisY);
    pAxisY->setLabelFormat("%d  ");

    return pChart;
}


void
ChartWindow::newScore(int team0Score, int team1Score, int iSet) {
    int iMax = std::max(team0Score, team1Score);
    QChart* pChart = chartVector.at(iSet);
    QLineSeries* pScoreSequence;
    pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->append(QPointF(iMax, team0Score));
    pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->append(QPointF(iMax, team1Score));
    if(iMax > maxScore) {
        maxScore = iMax;
        pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxScore);
        pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxScore);
    }
}


void
ChartWindow::resetScore(int iSet) {
    QChart* pChart = chartVector.at(iSet);
    QLineSeries* pScoreSequence;
    pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->clear();
    pScoreSequence->append(QPointF(0.0, 0.0));
    pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->append(QPointF(0, 0));
    maxScore = 25;
    pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxScore);
    pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxScore);
}


void
ChartWindow::resetAll() {
    maxScore = 25;
    for(int iSet=0; iSet<5; iSet++) {
        QChart* pChart = chartVector.at(iSet);
        QLineSeries* pScoreSequence;
        pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(0));
        pScoreSequence->clear();
        pScoreSequence->append(QPointF(0.0, 0.0));
        pScoreSequence = static_cast<QLineSeries*>(pChart->series().at(1));
        pScoreSequence->append(QPointF(0, 0));
        pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxScore);
        pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxScore);
    }
}
