#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Pole : protected QOpenGLFunctions
{
public:
    Pole(float height, float diameter);
    virtual ~Pole();

    void draw(QOpenGLShaderProgram *program);
    void updateStatus(float deltaTime);

private:
    void initGeometry(float height, float diameter);

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
