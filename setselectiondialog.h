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

#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QDialogButtonBox>


class SetSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    SetSelectionDialog();
    bool isPlotSelected();

public slots:
    void onSetSelected(bool);
    void onOk();
    void onCancel();

public:
    int iSelectedSet;

private:
    QRadioButton setSelected[5];
    QCheckBox buttonPlot;
    QDialogButtonBox* pButtonBox;
};

