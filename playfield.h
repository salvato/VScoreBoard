#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class PlayField : protected QOpenGLFunctions_3_3_Core
{
public:
    PlayField(float xField, float zField);
    ~PlayField();

public:
    void draw(QOpenGLShaderProgram* pProgram);

protected:
    QOpenGLBuffer fieldBuf;
};
