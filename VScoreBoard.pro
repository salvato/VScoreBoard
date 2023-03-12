QT += core
QT += gui
QT += widgets
QT += opengl
QT += openglwidgets


CONFIG += c++17

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    button.cpp \
    edit.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    scorecontroller.cpp \
    scorepanel.cpp \
    slidewidget.cpp \
    timeoutwindow.cpp \
    utility.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp \
    volleypanel.cpp

HEADERS += \
    button.h \
    edit.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    panelorientation.h \
    scorecontroller.h \
    scorepanel.h \
    slidewidget.h \
    timeoutwindow.h \
    utility.h \
    volleyapplication.h \
    volleycontroller.h \
    volleypanel.h

TRANSLATIONS += \
    VScoreBoard_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VScoreBoard.qrc \
    shaders.qrc

DISTFILES += \
    fshader.glsl \
    fshaderFade.glsl \
    fshaderFold.glsl \
    gl-transitions.txt \
    vshader.glsl \
    vshaderFade.glsl \
    vshaderFold.glsl
