#pragma once

#include "sphere.h"



class Avatar : public Sphere
{
public:
    Avatar(float _radius = 1.0,
           QVector3D   _position = QVector3D(0.0f, 0.0f, 0.0f),
           QQuaternion _rotation = QQuaternion(),
           QVector3D   _scale    = QVector3D(1.0f, 1.0f, 1.0f),
           QVector3D   _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    void setScale(QVector3D newScale);
    QVector3D getScale();
    QVector3D getPos();
    void setPos(QVector3D newPos);
    QVector3D getSpeed();
    void setSpeed(QVector3D newSpeed);
    QQuaternion getRotation();
    void setRotation(QQuaternion newRotation);
    void updateStatus(float deltaTime);
    QMatrix4x4 modelMatrix();

private:
    float       radius;
    QVector3D   position;
    QQuaternion rotation;
    QVector3D   scale;
    QVector3D   speed;
};
