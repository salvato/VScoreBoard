#pragma once

#include "object.h"

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Pole : public Object, protected QOpenGLFunctions
{
public:
    Pole(QSizeF      _size,
         QVector3D   _position = QVector3D(0.0f, 0.0f, 0.0f),
         QQuaternion _rotation = QQuaternion(),
         QVector3D   _scale    = QVector3D(1.0f, 1.0f, 1.0f),
         QVector3D   _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~Pole();

public:
    void draw(QOpenGLShaderProgram* pProgram);

private:
    void initGeometry(float height, float diameter);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
