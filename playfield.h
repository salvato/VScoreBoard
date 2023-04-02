#pragma once

#include "object.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class PlayField : public Object, protected QOpenGLFunctions_3_3_Core
{
public:
    PlayField(QSizeF      _size,
              QVector3D   _position = QVector3D(0.0f, 0.0f, 0.0f),
              QQuaternion _rotation = QQuaternion(),
              QVector3D   _scale    = QVector3D(1.0f, 1.0f, 1.0f),
              QVector3D   _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~PlayField();

public:
    void draw(QOpenGLShaderProgram* pProgram) override;

protected:
    QOpenGLBuffer fieldBuf;
};
