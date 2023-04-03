#include "particle.h"

#include <QOpenglTexture>


Particle::Particle(QVector4D   _color,
                   float       _life,
                   QSizeF      _size,
                   QVector3D   _position,
                   QQuaternion _rotation,
                   QVector3D   _scale,
                   QVector3D   _speed)
    : Object(_position,
             _rotation,
             _scale,
             _speed)
    , color(_color)
    , life(_life)
    , size(_size)
{
}


Particle::~Particle() {
    arrayBuf.destroy();
    indexBuf.destroy();
}


void
Particle::initGeometry(float height, float diameter) {
    (void)height;
    (void)diameter;
}


void
Particle::draw(QOpenGLShaderProgram* pProgram) {
    (void)pProgram;
}
