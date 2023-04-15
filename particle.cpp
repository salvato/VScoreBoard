#include "particle.h"

#include <QOpenglTexture>


Particle::Particle(QVector4D             _color,
                   float                 _life,
                   QSizeF                _size,
                   QOpenGLShaderProgram* _pProgram,
                   QOpenGLTexture*       _pTexture,
                   QVector3D             _position,
                   QQuaternion           _rotation,
                   QVector3D             _scale,
                   QVector3D             _speed)
    : Pole(_size,
           _pProgram,
           _pTexture,
           _position,
           _rotation,
           _scale,
           _speed)
    , color(_color)
    , life(_life)
    , size(_size)
{
}


Particle::~Particle() {
}

