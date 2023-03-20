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
#include "setselectiondialog.h"

#include <QIcon>
#include <QGridLayout>


SetSelectionDialog::SetSelectionDialog()
    : QDialog()
    , iSelectedSet(0)
{
    setWindowTitle("Andamento Set");
    setWindowIcon(QIcon(":/buttonIcons/Plot.png"));
    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
    QGridLayout* pLayout = new QGridLayout();
    for(int i=0; i<5; i++) {
        setSelected[i].setText(QString("Set %1").arg(i+1));
        connect(&setSelected[i], SIGNAL(toggled(bool)),
                this, SLOT(onSetSelected(bool)));
        pLayout->addWidget(&setSelected[i], i, 0, 1, 1);
    }
    pLayout->addWidget(pButtonBox, 5, 0, 1, 3);
    setLayout(pLayout);
    setSelected[iSelectedSet].setChecked(true);
}


void
SetSelectionDialog::onSetSelected(bool) {
    iSelectedSet = 0;
    for(int i=1; i<5; i++) {
        if(setSelected[i].isChecked()) {
            iSelectedSet = i;
            break;
        }
    }
}


void
SetSelectionDialog::onOk() {
    accept();
}


void
SetSelectionDialog::onCancel() {
    reject();
}
