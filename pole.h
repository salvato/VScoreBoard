#pragma once

#include "object.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Pole : public Object, protected QOpenGLFunctions_3_3_Core
{
public:
    Pole(QSizeF          _size,
         QOpenGLTexture* _pTexture = nullptr,
         QVector3D       _position = QVector3D(0.0f, 0.0f, 0.0f),
         QQuaternion     _rotation = QQuaternion(),
         QVector3D       _scale    = QVector3D(1.0f, 1.0f, 1.0f),
         QVector3D       _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~Pole();

public:
    void draw(QOpenGLShaderProgram* pProgram) override;

private:
    void initGeometry(float height, float diameter);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};
