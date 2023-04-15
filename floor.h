#pragma once

#include "object.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class Floor : public Object, protected QOpenGLFunctions_3_3_Core
{
public:
    Floor(QSizeF                _size,
          QOpenGLShaderProgram* _pProgram = nullptr,
          QOpenGLTexture*       _pTexture = nullptr,
          QVector3D             _position = QVector3D(0.0f, 0.0f, 0.0f),
          QQuaternion           _rotation = QQuaternion(),
          QVector3D             _scale    = QVector3D(1.0f, 1.0f, 1.0f),
          QVector3D             _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~Floor();

public:
    void draw() override;
    void draw(QOpenGLShaderProgram* pOtherProgram);

private:
    QOpenGLBuffer floorBuf;
};
