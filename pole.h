#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class Pole : protected QOpenGLFunctions_3_3_Core
{
public:
    Pole(float height, float diameter);
    ~Pole();

public:
    void draw(QOpenGLShaderProgram* pProgram);

protected:
    QOpenGLBuffer* pPoleBuf;
    QOpenGLBuffer indexBuf;

private:
    int nverts;
};
