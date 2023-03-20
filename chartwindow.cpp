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

    // TODO:
    // Create charts for each Game Set (should be parametrized ?)
    for(int i=0; i<5; i++) {
        QChart* pChart = createLineChart();
        pChart->setTitle(tr("Andamento Set %1").arg(i+1));
        // The ownership of the chart is passed to the QChartView()
        QChartView* pChartView = new QChartView(pChart);
        pChartViews.append(pChartView);
    }
    auto* pLayout = new QGridLayout();
    pLayout->addWidget(pChartViews.at(0), 0, 0, 1, 1);
    setLayout(pLayout);
}


QChart*
ChartWindow::createLineChart() {
    QChart* pChart = new QChart();
    pChart->setTheme(QChart::ChartThemeBlueCerulean);
    QFont font = pChart->titleFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pChart->setTitleFont(font);
    pChart->setFont(font);
    pChart->legend()->setFont(font);

    QValueAxis* pAxisX = new QValueAxis();
    pAxisX->setRange(0, maxX);
    pAxisX->setTickCount(2);
    font = pAxisX->labelsFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pAxisX->setLabelsFont(font);
    pAxisX->setLabelFormat("%d");

    // Add space to label to add space between labels and vertical axis
    QValueAxis* pAxisY = new QValueAxis();
    pAxisY->setRange(0, maxY);
    pAxisY->setTickCount(2);
    font = pAxisY->labelsFont();
    font.setPointSize(3*font.pointSize());
    font.setBold(true);
    pAxisY->setLabelsFont(font);
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
    for(int i=0; i<5; i++) {
        QChart* pChart = pChartViews.at(i)->chart();
        QLineSeries* pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(iTeam));
        pScoreSequence->setName(sLabel);
    }
    update();
}


void
ChartWindow::updateScore(int team0Score, int team1Score, int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    int yMax = std::max(team0Score, team1Score);
    int xMax = team0Score+team1Score;
    QLineSeries* pScoreSequence;
    QChart* pChart = pChartViews.at(iSet)->chart();
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->append(xMax, team0Score);
    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->append(xMax, team1Score);
    if(xMax > maxX) {
        maxX = xMax;
        pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxX);
    }
    if(yMax > maxY) {
        maxY = yMax;
        pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxY);
    }
    update();
}


void
ChartWindow::resetScore(int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    maxX = maxY = 25;
    QChart* pChart = pChartViews.at(iSet)->chart();
    QLineSeries* pScoreSequence;

    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(0));
    pScoreSequence->clear();
    pScoreSequence->append(0, 0);

    pScoreSequence = reinterpret_cast<QLineSeries*>(pChart->series().at(1));
    pScoreSequence->clear();
    pScoreSequence->append(0, 0);

    pChart->axes(Qt::Horizontal).constFirst()->setRange(0, maxX);
    pChart->axes(Qt::Vertical).constFirst()->setRange(0, maxY);
    update();
}


void
ChartWindow::resetAll() {
    maxX = maxY = 25;
    for(int i=0; i<pChartViews.count(); i++) {
        resetScore(i);
    }
    update();
}


void
ChartWindow::show(int iSet) {
    QChartView* pChartView = pChartViews.at(iSet);
    auto* pLayout = reinterpret_cast<QGridLayout*>(layout());
    pLayout->invalidate();
    pLayout->addWidget(pChartView, 0, 0, 1, 1);
    setLayout(pLayout);
    QWidget::show();
}


void
ChartWindow::showMaximized(int iSet) {
    QChartView* pChartView = pChartViews.at(iSet);
    auto* pLayout = reinterpret_cast<QGridLayout*>(layout());
    pLayout->invalidate();
    pLayout->addWidget(pChartView, 0, 0, 1, 1);
    setLayout(pLayout);
    QWidget::showMaximized();
}


void
ChartWindow::showFullScreen(int iSet) {
    QChartView* pChartView = pChartViews.at(iSet);
    pChartView->chart()->legend()->setReverseMarkers(iSet % 2);
    auto* pLayout = reinterpret_cast<QGridLayout*>(layout());
    pLayout->invalidate();
    pLayout->addWidget(pChartView, 0, 0, 1, 1);
    setLayout(pLayout);
    QWidget::showFullScreen();
}


void
ChartWindow::hide() {
    QWidget::hide();
}
