#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class WhiteLine : protected QOpenGLFunctions_3_3_Core
{
public:
    WhiteLine(float xDim, float zDim);
    ~WhiteLine();

public:
    void draw(QOpenGLShaderProgram* pProgram);

protected:
    QOpenGLBuffer lineBuf;
};
