#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class TeamAvatar : protected QOpenGLFunctions
{
public:
    TeamAvatar();
    virtual ~TeamAvatar();

    void drawAvatar(QOpenGLShaderProgram* pProgram);

private:
    void initAvatar();

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
