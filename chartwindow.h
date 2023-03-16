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


QT_FORWARD_DECLARE_CLASS(QChart)
QT_FORWARD_DECLARE_CLASS(QLineSeries)


typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;


class ChartWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWindow(QWidget *parent = nullptr);
    void newScore(int team0Score, int team1Score, int iSet);
    void resetScore(int iSet);
    void resetAll();

signals:

private:
    QChart* createLineChart();

private:
    QPalette        panelPalette;
    QLinearGradient panelGradient;
    QBrush          panelBrush;

    int              maxScore;
    QVector<QChart*> chartVector;
};
