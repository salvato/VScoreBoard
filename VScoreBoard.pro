QT += core
QT += gui
QT += widgets
#QT += multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    button.cpp \
    cameratab.cpp \
    edit.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    scorecontroller.cpp \
    scorepanel.cpp \
    slidewindow.cpp \
    timeoutwindow.cpp \
    utility.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp \
    volleypanel.cpp \
    vscoreboard.cpp

HEADERS += \
    button.h \
    cameratab.h \
    edit.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    panelorientation.h \
    scorecontroller.h \
    scorepanel.h \
    slidewindow.h \
    timeoutwindow.h \
    utility.h \
    volleyapplication.h \
    volleycontroller.h \
    volleypanel.h \
    vscoreboard.h

TRANSLATIONS += \
    VScoreBoard_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VScoreBoard.qrc
