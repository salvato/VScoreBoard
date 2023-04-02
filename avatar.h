#pragma once

#include "sphere.h"
#include "object.h"


class Avatar : public Object, public Sphere
{
public:
    Avatar(float _radius = 1.0,
           QVector3D   _position = QVector3D(0.0f, 0.0f, 0.0f),
           QQuaternion _rotation = QQuaternion(),
           QVector3D   _scale    = QVector3D(1.0f, 1.0f, 1.0f),
           QVector3D   _speed    = QVector3D(0.0f, 0.0f, 0.0f));

public:
    void updateStatus(float deltaTime) override;

private:
    float       radius;
};
