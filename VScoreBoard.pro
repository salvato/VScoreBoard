#Copyright (C) 2023  Gabriele Salvato

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT += core
QT += gui
QT += widgets
QT += opengl
QT += openglwidgets
QT += charts


CONFIG += c++17

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    avatar.cpp \
    button.cpp \
    chartwindow.cpp \
    edit.cpp \
    floor.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    object.cpp \
    playfield.cpp \
    pole.cpp \
    racewidget.cpp \
    racewindow.cpp \
    scorecontroller.cpp \
    scorepanel.cpp \
    setselectiondialog.cpp \
    slidewidget.cpp \
    sphere.cpp \
    timeoutwindow.cpp \
    utility.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp \
    volleypanel.cpp \
    whiteline.cpp

HEADERS += \
    avatar.h \
    button.h \
    chartwindow.h \
    edit.h \
    floor.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    object.h \
    panelorientation.h \
    playfield.h \
    pole.h \
    racewidget.h \
    racewindow.h \
    scorecontroller.h \
    scorepanel.h \
    setselectiondialog.h \
    slidewidget.h \
    sphere.h \
    timeoutwindow.h \
    utility.h \
    volleyapplication.h \
    volleycontroller.h \
    volleypanel.h \
    whiteline.h

TRANSLATIONS += VScoreBoard_en_US.ts
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
    fAngular.glsl \
    fBookFlip.glsl \
    fshader.glsl \
    fshaderFade.glsl \
    fshaderFold.glsl \
    vshader.glsl \
    vshaderFade.glsl \
    vshaderFold.glsl

win32:RC_ICONS += Logo.ico
