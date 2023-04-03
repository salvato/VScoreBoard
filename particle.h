#pragma once

#include "object.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Particle : public Object, protected QOpenGLFunctions_3_3_Core
{
public:
    Particle(QVector4D   _color    = QVector4D(1.0f, 1.0f, 1.0f, 1.0f),
             float       _life     = 1.0f,
             QSizeF      _size     = QSizeF(1.0f, 1.0f),
             QVector3D   _position = QVector3D(0.0f, 0.0f, 0.0f),
             QQuaternion _rotation = QQuaternion(),
             QVector3D   _scale    = QVector3D(1.0f, 1.0f, 1.0f),
             QVector3D   _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~Particle();

public:
    void draw(QOpenGLShaderProgram* pProgram) override;

public:
    QVector4D color;
    float life;

private:
    void initGeometry(float height, float diameter);

private:
    QSizeF size;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
