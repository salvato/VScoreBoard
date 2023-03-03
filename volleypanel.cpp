#include <QtGlobal>
#include <QtWidgets>
#include <QProcess>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>

#include "volleypanel.h"
#include "timeoutwindow.h"
#include "utility.h"

VolleyPanel::VolleyPanel(QFile *myLogFile, QWidget *parent)
    : ScorePanel(myLogFile, parent)
    , sLeftLogo(QString(":/Logo_UniMe.png"))
    , sRightLogo(QString(":/Logo_SSD_UniMe.png"))
    , iServizio(0)
    , maxTeamNameLen(15)
    , pTimeoutWindow(Q_NULLPTR)
{
    pPixmapLeftTop  = new QPixmap(sLeftLogo);
    pPixmapRightTop = new QPixmap(sRightLogo);

    sFontName = QString("Liberation Sans Bold");
    fontWeight = QFont::Black;

    QSize panelSize = QGuiApplication::primaryScreen()->geometry().size();
    iTeamFontSize    = std::min(panelSize.height()/8,
                                int(panelSize.width()/(2.2*maxTeamNameLen)));
    iScoreFontSize   = std::min(panelSize.height()/4,
                                int(panelSize.width()/9));
    iLabelsFontSize  = panelSize.height()/8; // 2 Righe
    iTimeoutFontSize = panelSize.height()/8; // 2 Righe
    iSetFontSize     = panelSize.height()/8; // 2 Righe

    // QWidget propagates explicit palette roles from parent to child.
    // If you assign a brush or color to a specific role on a palette and
    // assign that palette to a widget, that role will propagate to all
    // the widget's children, overriding any system defaults for that role.
    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
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


    pTimeoutWindow = new TimeoutWindow(Q_NULLPTR);
    connect(pTimeoutWindow, SIGNAL(doneTimeout()),
            this, SLOT(onTimeoutDone()));

    createPanelElements();
    buildLayout();
}


VolleyPanel::~VolleyPanel() {
    if(pTimeoutWindow) delete pTimeoutWindow;
    pTimeoutWindow = nullptr;
}


void
VolleyPanel::setTeam(int iTeam, QString sTeamName) {
    pTeam[iTeam]->setText(sTeamName.left(maxTeamNameLen));
}


void
VolleyPanel::setScore(int iTeam, int iScore) {
    pScore[iTeam]->setText(QString("%1").arg(iScore));
}


void
VolleyPanel::setSets(int iTeam, int iSets) {
    pSet[iTeam]->setText(QString("%1").arg(iSets));
}


void
VolleyPanel::setServizio(int iServizio) {
    servizio[0]->setText(" ");
    servizio[1]->setText(" ");
    if(iServizio == 0) {
        servizio[0]->setPixmap(*pPixmapService);
    } else if(iServizio == 1) {
        servizio[1]->setPixmap(*pPixmapService);
    }
}


void
VolleyPanel::setTimeout(int iTeam, int iTimeout) {
    timeout[iTeam]->setText(QString("%1").arg(iTimeout));
}


void
VolleyPanel::startTimeout(int iTimeoutDuration) {
    pTimeoutWindow->startTimeout(iTimeoutDuration*1000);
    pTimeoutWindow->showFullScreen();
}


void
VolleyPanel::stopTimeout() {
    pTimeoutWindow->stopTimeout();
    pTimeoutWindow->hide();
}


void
VolleyPanel::setMirrored(bool isPanelMirrored) {
    isMirrored = isPanelMirrored;
    buildLayout();
}


bool
VolleyPanel::getMirrored() {
    return isMirrored;
}


void
VolleyPanel::closeEvent(QCloseEvent *event) {
    if(pTimeoutWindow) delete pTimeoutWindow;
    pTimeoutWindow = nullptr;
    ScorePanel::closeEvent(event);
    event->accept();
}


void
VolleyPanel::onTimeoutDone() {
    showFullScreen();
    pTimeoutWindow->hide();
}


void
VolleyPanel::createPanelElements() {
    // QWidget propagates explicit palette roles from parent to child.
    // If you assign a brush or color to a specific role on a palette and
    // assign that palette to a widget, that role will propagate to all
    // the widget's children, overriding any system defaults for that role.

    // Timeout
    timeoutLabel = new QLabel("Timeout");
    timeoutLabel->setFont(QFont(sFontName, iLabelsFontSize/2, fontWeight));
    timeoutLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    for(int i=0; i<2; i++) {
        timeout[i] = new QLabel("8");
        timeout[i]->setFrameStyle(QFrame::NoFrame);
        timeout[i]->setFont(QFont(sFontName, iTimeoutFontSize, fontWeight));
    }

    // Set
    pSetLabel = new QLabel(tr("Set"));
    pSetLabel->setFont(QFont(sFontName, iLabelsFontSize/2, fontWeight));
    pSetLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    for(int i=0; i<2; i++) {
        pSet[i] = new QLabel("8");
        pSet[i]->setFrameStyle(QFrame::NoFrame);
        pSet[i]->setFont(QFont(sFontName, iSetFontSize, fontWeight));
    }

    // Score
    pScoreLabel = new QLabel(tr(""));
    pScoreLabel->setFont(QFont(sFontName, iLabelsFontSize, fontWeight));
    pScoreLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    for(int i=0; i<2; i++){
        pScore[i] = new QLabel("88");
        pScore[i]->setAlignment(Qt::AlignHCenter);
        pScore[i]->setFont(QFont(sFontName, iScoreFontSize, fontWeight));
        pScore[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // Servizio
    for(int i=0; i<2; i++){
        servizio[i] = new QLabel(" ");
        servizio[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // Teams
    QPalette pal = panelPalette;
    pal.setColor(QPalette::WindowText, Qt::white);
    for(int i=0; i<2; i++) {
        pTeam[i] = new QLabel();
        pTeam[i]->setPalette(pal);
        pTeam[i]->setFont(QFont(sFontName, iTeamFontSize, fontWeight));
        pTeam[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    pTeam[0]->setText(tr("Locali"));
    pTeam[1]->setText(tr("Ospiti"));
}


void
VolleyPanel::setLeftLogo(QString sFileLogo) {
    if(QFile::exists(sFileLogo)) {
        sLeftLogo = sFileLogo;
        if(pPixmapLeftTop) delete pPixmapLeftTop;
        pPixmapLeftTop = new QPixmap(sLeftLogo);
        buildLayout();
    }
}


void
VolleyPanel::setRightLogo(QString sFileLogo) {
    if(QFile::exists(sFileLogo)) {
        sRightLogo = sFileLogo;
        if(pPixmapRightTop) delete pPixmapRightTop;
        pPixmapRightTop = new QPixmap(sRightLogo);
        buildLayout();
    }
}


QGridLayout*
VolleyPanel::createPanel() {
    QGridLayout *layout = new QGridLayout();

    int ileft  = 0;
    int iright = 1;
    if(isMirrored) {
        ileft  = 1;
        iright = 0;
    }
    QLabel* leftTopLabel = new QLabel();
    leftTopLabel->setPixmap(*pPixmapLeftTop);

    QLabel* rightTopLabel = new QLabel();
    rightTopLabel->setPixmap(*pPixmapRightTop);

    pPixmapService = new QPixmap(":/ball2.png");
    *pPixmapService = pPixmapService->scaled(2*iLabelsFontSize/3, 2*iLabelsFontSize/3);

    layout->addWidget(pTeam[ileft],     0, 0, 2, 6, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pTeam[iright],    0, 6, 2, 6, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(pScore[ileft],    2, 1, 4, 3, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(servizio[ileft],  2, 4, 4, 1, Qt::AlignLeft   |Qt::AlignTop);
    layout->addWidget(pScoreLabel,      2, 5, 4, 2, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(servizio[iright], 2, 7, 4, 1, Qt::AlignRight  |Qt::AlignTop);
    layout->addWidget(pScore[iright],   2, 8, 4, 3, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(pSet[ileft],      6, 2, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pSetLabel,        6, 3, 2, 6, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(pSet[iright],     6, 9, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);

    layout->addWidget(leftTopLabel,     8, 0, 2, 2, Qt::AlignLeft   |Qt::AlignBottom);
    layout->addWidget(timeout[ileft],   8, 2, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(timeoutLabel,     8, 3, 2, 6, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(timeout[iright],  8, 9, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget(rightTopLabel,    8,10, 2, 2, Qt::AlignRight  |Qt::AlignBottom);

    return layout;
}


void
VolleyPanel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("%1  %2")
                   .arg(pSetLabel->text(), pScoreLabel->text()));
#endif
        pSetLabel->setText(tr("Set"));
        pScoreLabel->setText(tr("Punti"));
    } else
        QWidget::changeEvent(event);
}

