
#pragma once

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class QOpenGLShaderProgram;
class QOpenGLTexture;

class Object
{
public:
    Object(QOpenGLShaderProgram* _pProgram = nullptr,
           QOpenGLTexture*       _pTexture = nullptr,
           QVector3D             _position = QVector3D(0.0f, 0.0f, 0.0f),
           QQuaternion           _rotation = QQuaternion(),
           QVector3D             _scale    = QVector3D(1.0f, 1.0f, 1.0f),
           QVector3D             _speed    = QVector3D(0.0f, 0.0f, 0.0f));

public:
    void setScale(QVector3D newScale);
    QVector3D getScale();
    QVector3D getPos();
    void setPos(QVector3D newPos);
    QVector3D getSpeed();
    void setSpeed(QVector3D newSpeed);
    QQuaternion getRotation();
    void setRotation(QQuaternion newRotation);
    virtual void updateStatus(float deltaTime);
    QMatrix4x4 modelMatrix();
    virtual void draw();
    virtual void draw(QOpenGLShaderProgram* pOtherProgram);

protected:
    QOpenGLShaderProgram* pProgram;
    QOpenGLTexture* pTexture;
    QVector3D   position;
    QQuaternion rotation;
    QVector3D   scale;
    QVector3D   speed;
};
