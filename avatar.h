#pragma once

#include "sphere.h"



class Avatar : public Sphere
{
public:
    Avatar(float radius);
    QVector3D getPos();
    void setPos(QVector3D newPos);
    QVector3D getSpeed();
    void setSpeed(QVector3D newSpeed);

private:
    QVector3D position;
    QVector3D speed;
};
