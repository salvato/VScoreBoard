#include "avatar.h"


Avatar::Avatar(float radius)
    : Sphere(radius, 40, 40)
    , position(QVector3D(0.0f, 0.0f, 0.0f))
    , speed(QVector3D(0.0f, 0.0f, 0.0f))
{
}

QVector3D
Avatar::getPos() {
    return position;
}


void
Avatar::setPos(QVector3D newPos) {
    position = newPos;
}


QVector3D
Avatar::getSpeed(){
    return speed;
}


void
Avatar::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}
