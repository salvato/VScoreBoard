#include "avatar.h"


Avatar::Avatar(float _radius,
               QVector3D _position,
               QQuaternion _rotation,
               QVector3D _speed)
    : Sphere(_radius, 40, 40)
    , radius(_radius)
    , position(_position)
    , rotation(_rotation)
    , speed(_speed)
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


QQuaternion
Avatar::getRotation() {
    return rotation;
}


void
Avatar::setRotation(QQuaternion newRotation) {
    rotation = newRotation;
}


QVector3D
Avatar::getSpeed(){
    return speed;
}


void   
Avatar::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}


void
Avatar::updateStatus(float deltaTime) {
    position += speed*deltaTime;
    QVector3D angle = speed*(deltaTime/m_radius);
    QQuaternion qx = QQuaternion::fromAxisAndAngle(QVector3D( 0.0f,  0.0f, -1.0f), qRadiansToDegrees(angle.x()));
    QQuaternion qy = QQuaternion::fromAxisAndAngle(QVector3D(-1.0f,  0.0f,  0.0f), qRadiansToDegrees(angle.y()));
    QQuaternion qz = QQuaternion::fromAxisAndAngle(QVector3D( 1.0f,  0.0f,  0.0f), qRadiansToDegrees(angle.z()));
    rotation *= qx * qy * qz;
}


QMatrix4x4
Avatar::modelMatrix() {
    QMatrix4x4 M;
    M.setToIdentity();
    M.translate(position);
    M.rotate(rotation);
    return M;
}
