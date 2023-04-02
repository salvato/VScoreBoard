#include "object.h"

Object::Object(QVector3D   _position,
               QQuaternion _rotation,
               QVector3D   _scale,
               QVector3D   _speed)
    : position(_position)
    , rotation(_rotation)
    , scale(_scale)
    , speed(_speed)
{
}


QVector3D
Object::getPos() {
    return position;
}


void
Object::setPos(QVector3D newPos) {
    position = newPos;
}


QQuaternion
Object::getRotation() {
    return rotation;
}


void
Object::setRotation(QQuaternion newRotation) {
    rotation = newRotation;
}


QVector3D
Object::getSpeed(){
    return speed;
}


void
Object::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}


void
Object::updateStatus(float deltaTime) {
    (void)deltaTime;
}


void
Object::setScale(QVector3D newScale) {
    scale = newScale;
}


QVector3D
Object::getScale() {
    return scale;
}


QMatrix4x4
Object::modelMatrix() {
    QMatrix4x4 M;
    M.setToIdentity();
    M.translate(position);
    M.rotate(rotation);
    M.scale(scale);
    return M;
}