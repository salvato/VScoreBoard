#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class Floor : protected QOpenGLFunctions_3_3_Core
{
public:
    Floor(int xFloor, int zFloor);
    ~Floor();

public:
    void draw(QOpenGLShaderProgram* pProgram);

protected:
    QOpenGLBuffer floorBuf;
};
