#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

#include "volleyapplication.h"
#include "volleycontroller.h"
#include "utility.h"

VolleyApplication::VolleyApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , pLogFile(nullptr)
    , pScoreController(nullptr)
{
    pSettings = new QSettings("Gabriele Salvato", "Volley ScoreBoard");
    sLanguage = pSettings->value("language/current",  QString("Italiano")).toString();

    QString sBaseDir;
    sBaseDir = QDir::homePath();
    if(!sBaseDir.endsWith(QString("/"))) sBaseDir+= QString("/");
    logFileName = QString("%1volley_panel.txt").arg(sBaseDir);
    PrepareLogFile();

#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Initial Language: %1").arg(sLanguage));
#endif

    if(sLanguage == QString("English")) {
        if(Translator.load(":/VolleyPanel_en_US.ts"))
            QCoreApplication::installTranslator(&Translator);
    }

    // Initialize the random number generator
    QTime time(QTime::currentTime());
    srand(uint(time.msecsSinceStartOfDay()));

    pScoreController = new VolleyController(pLogFile);
    pScoreController->show();
}


bool
VolleyApplication::PrepareLogFile() {
//#ifdef LOG_MESG
//    QFileInfo checkFile(logFileName);
//    if(checkFile.exists() && checkFile.isFile()) {
//        QDir renamed;
//        renamed.remove(logFileName+QString(".bkp"));
//        renamed.rename(logFileName, logFileName+QString(".bkp"));
//    }
//    pLogFile = new QFile(logFileName);
//    if (!pLogFile->open(QIODevice::WriteOnly)) {
//        QMessageBox::information(Q_NULLPTR, "Segnapunti Volley",
//                                 QString("Impossibile aprire il file %1: %2.")
//                                 .arg(logFileName).arg(pLogFile->errorString()));
//        delete pLogFile;
//        pLogFile = nullptr;
//    }
//#endif
    return true;
}
