#include "racewindow.h"
#include "utility.h"

#include <QIcon>
#include <QGridLayout>
#include <QLabel>

RaceWindow::RaceWindow(QWidget *parent)
    : QWidget{parent}
{
    setWindowIcon(QIcon(":/buttonIcons/plot.png"));
    setWindowTitle("RaceWindow");

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);

    panelPalette.setColor(QPalette::WindowText,      Qt::white);
    panelPalette.setColor(QPalette::Base,            Qt::black);
    panelPalette.setColor(QPalette::AlternateBase,   Qt::blue);
    panelPalette.setColor(QPalette::Text,            Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,      Qt::white);
    panelPalette.setColor(QPalette::HighlightedText, Qt::gray);
    panelPalette.setColor(QPalette::Highlight,       Qt::transparent);

    setPalette(panelPalette);

    QFont font;
    int iFontSize;
    int hMargin, vMargin;
    QMargins margins;

    font = labelTeam0.font();
    font.setCapitalization(QFont::Capitalize);
    margins = labelTeam0.contentsMargins();
    vMargin = margins.bottom() + margins.top();
    hMargin = margins.left() + margins.right();
    iFontSize = qMin((labelTeam0.width()/MAX_NAMELENGTH)-2*hMargin,
                     labelTeam0.height()-vMargin);
    font.setPixelSize(iFontSize);
    font.setWeight(QFont::Black);
    labelScore0.setFont(font);
    labelScore1.setFont(font);
    font.setPixelSize(iFontSize*0.75);
    labelTeam0.setFont(font);
    labelTeam1.setFont(font);

    sTeamName[0] = "Locali";
    sTeamName[1] = "Ospiti";

    QPixmap pixmap0(":/ball0.png");
    pixmap0 = pixmap0.scaledToWidth(iFontSize);
    QPixmap pixmap1(":/ball1.png");
    pixmap1 = pixmap1.scaledToWidth(iFontSize);
    pRaceWidget = new RaceWidget();
    connect(pRaceWidget, SIGNAL(raceDone()),
            this, SIGNAL(raceDone()));
    connect(pRaceWidget, SIGNAL(newScore(int,int)),
            this, SLOT(onNewScore(int,int)));

    QGridLayout* pLayout = new QGridLayout();

    labelTeam0.setText(sTeamName[0]);
    labelTeam1.setText(sTeamName[1]);
    labelIcon0.setPixmap(pixmap0);
    labelIcon1.setPixmap(pixmap1);
    labelScore0.setText("0");
    labelScore1.setText("0");

    pLayout->addWidget(&labelIcon0,  0, 0, 1, 1, Qt::AlignRight);
    pLayout->addWidget(&labelTeam0,  0, 1, 1, 1, Qt::AlignLeft);
    pLayout->addWidget(&labelScore0, 0, 2, 1, 1, Qt::AlignHCenter);
    pLayout->addWidget(&labelScore1, 0, 3, 1, 1, Qt::AlignHCenter);
    pLayout->addWidget(&labelTeam1,  0, 4, 1, 1, Qt::AlignRight);
    pLayout->addWidget(&labelIcon1,  0, 5, 1, 1, Qt::AlignLeft);
    pLayout->addWidget(pRaceWidget,  1, 0, 6, 6);

    setLayout(pLayout);

    timerStart.setSingleShot(true);
    connect(&timerStart, SIGNAL(timeout()),
            this, SLOT(onTimeToStartRace()));
}

void
RaceWindow::closeEvent(QCloseEvent* event) {
    event->accept();
}


void
RaceWindow::updateLabel(int iTeam, QString sLabel) {
    pRaceWidget->updateLabel(iTeam, sLabel);
}


void
RaceWindow::updateScore(int team0Score, int team1Score, int iSet) {
    pRaceWidget->updateScore(team0Score, team1Score, iSet);
}


void
RaceWindow::resetScore(int iSet) {
    pRaceWidget->resetScore(iSet);
}


void
RaceWindow::resetAll() {
    pRaceWidget->resetAll();
}


void
RaceWindow::startRace(int iSet) {
    iCurrentSet = iSet;
    pRaceWidget->resetInitialStatus();
    timerStart.start(3000);
}


void
RaceWindow::onTimeToStartRace() {
    pRaceWidget->startRace(iCurrentSet);
}


void
RaceWindow::onNewScore(int s0, int s1) {
    labelScore0.setText(QString("%1").arg(s0));
    labelScore1.setText(QString("%1").arg(s1));
}
