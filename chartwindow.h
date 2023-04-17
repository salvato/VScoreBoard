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

#include <QWidget>
#include <QTimer>


QT_FORWARD_DECLARE_CLASS(QChart)
QT_FORWARD_DECLARE_CLASS(QChartView)
QT_FORWARD_DECLARE_CLASS(QLineSeries)


typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;


class ChartWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWindow(QWidget *parent = nullptr);
    void updateScore(int team0Score, int team1Score, int iSet);
    void updateLabel(int iTeam, QString sLabel);
    void resetScore(int iSet);
    void resetAll();
    bool startChartAnimation(int iSet);
    void hide();

public slots:
    void onTimeToAnimate();
    void onTimeToClose();

signals:
    void done();

private:
    QChart* createLineChart();

private:
    QPalette        panelPalette;
    QLinearGradient panelGradient;
    QBrush          panelBrush;

    int                maxX;
    int                maxY;
    QChartView*        pChartView;
    QString            sTeamName[2];
    QVector<QVector2D> score[5];
    int                maxScore[5];
    int                indexScore;
    int                iCurrentSet;
    QTimer             animateTimer;
    QTimer             closeTimer;
//    QTimer timerRotate;
//    int iCurrentSet;
};
