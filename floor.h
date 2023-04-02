#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class Floor : protected QOpenGLFunctions_3_3_Core
{
public:
    Floor(QSizeF      _size,
          QVector3D   _position=QVector3D(0.0f, 0.0f, 0.0f),
          QQuaternion _rotation=QQuaternion(),
          QVector3D   _speed   =QVector3D(0.0f, 0.0f, 0.0f));
    ~Floor();

public:
    QVector3D getPos();
    void setPos(QVector3D newPos);
    QVector3D getSpeed();
    void setSpeed(QVector3D newSpeed);
    QQuaternion getRotation();
    void setRotation(QQuaternion newRotation);
    void updateStatus(float deltaTime);
    QMatrix4x4 modelMatrix();
    void draw(QOpenGLShaderProgram* pProgram);

private:
    QSizeF      size;
    QVector3D   position;
    QQuaternion rotation;
    QVector3D   speed;

protected:
    QOpenGLBuffer floorBuf;
};
