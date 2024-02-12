#include "avatar.h"

#include <QOpenglTexture>
#include <QtMath>


Avatar::Avatar(float                 _radius,
               QOpenGLShaderProgram* _pProgram,
               QOpenGLTexture*       _pTexture,
               QVector3D             _position,
               QQuaternion           _rotation,
               QVector3D             _scale,
               QVector3D             _speed)
    : Object(_pProgram,
             _pTexture,
             _position,
             _rotation,
             _scale,
             _speed)
    , Sphere(_radius, 40, 40)
{
}


void
Avatar::draw() {
    pProgram->bind();
    if(pTexture)
        pTexture->bind();
    pProgram->setUniformValue("model", modelMatrix());
    Sphere::draw(pProgram);
}


void
Avatar::draw(QOpenGLShaderProgram *pOtherProgram) {
    pOtherProgram->bind();
    if(pTexture)
        pTexture->bind();
    pOtherProgram->setUniformValue("model", modelMatrix());
    Sphere::draw(pOtherProgram);
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

