#include "volleyapplication.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QSurfaceFormat>

int
main(int argc, char *argv[]) {
    qputenv("QT_LOGGING_RULES","*.debug=false;qt.qpa.*=false"); // supress anoying messages

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    VolleyApplication a(argc, argv);
    QString sVersion = QString("2.0");
    a.setApplicationVersion(sVersion);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "VScoreBoard_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    int iResult = a.exec();
    return iResult;
}
