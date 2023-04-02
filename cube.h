#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Cube : protected QOpenGLFunctions
{
public:
    Cube(float height, float diameter);
    virtual ~Cube();

    void draw(QOpenGLShaderProgram *program);

private:
    void initGeometry(float height, float diameter);

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
