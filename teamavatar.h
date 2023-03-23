#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class TeamAvatar : protected QOpenGLFunctions
{
public:
    TeamAvatar();
    virtual ~TeamAvatar();

public:
    void draw(QOpenGLShaderProgram* pProgram);

private:
    void init();

private:
    QOpenGLBuffer vertexBuf;
    QOpenGLBuffer indexBuf;
};
