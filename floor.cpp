#include "floor.h"

#include <QVector>
#include <QOpenglTexture>


Floor::Floor(QSizeF                _size,
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
{
    initializeOpenGLFunctions();

    float xFloor = _size.width();
    float zFloor = _size.height();

    QVector<float>  vertices;
    vertices <<
        // positions                   // normals              // texcoords
        -xFloor << 0.0f <<  zFloor <<  0.0f << 1.0f << 0.0f <<   0.0f <<   0.0f <<
         xFloor << 0.0f <<  zFloor <<  0.0f << 1.0f << 0.0f << xFloor <<   0.0f <<
        -xFloor << 0.0f << -zFloor <<  0.0f << 1.0f << 0.0f <<   0.0f << zFloor <<

         xFloor << 0.0f << -zFloor <<  0.0f << 1.0f << 0.0f << xFloor << zFloor <<
        -xFloor << 0.0f << -zFloor <<  0.0f << 1.0f << 0.0f <<   0.0f << zFloor <<
         xFloor << 0.0f <<  zFloor <<  0.0f << 1.0f << 0.0f << xFloor << 0.0f;

    // Transfer vertex data to VBO
    floorBuf.create();
    floorBuf.bind();
    floorBuf.allocate(vertices.data(), int(vertices.count()*sizeof(float)));
    floorBuf.release();
}


Floor::~Floor() {
    floorBuf.destroy();
}


void
Floor::draw() {
    draw(pProgram);
}


void
Floor::draw(QOpenGLShaderProgram* pOtherProgram)
{
    pOtherProgram->bind();
    if(pTexture)
        pTexture->bind();
    pOtherProgram->setUniformValue("model", modelMatrix());

    floorBuf.bind();

    // Offset for position
    quintptr offset = 0;
    pOtherProgram->enableAttributeArray("vPosition");
    pOtherProgram->setAttributeBuffer("vPosition", GL_FLOAT, offset, 3, 8*sizeof(float));

    // Offset for normal coordinate
    offset += 3*sizeof(float);
    pOtherProgram->enableAttributeArray("vNormal");
    pOtherProgram->setAttributeBuffer("vNormal", GL_FLOAT, offset, 3, 8*sizeof(float));

    // Offset for texture coordinate
    offset += 3*sizeof(float);
    pOtherProgram->enableAttributeArray("vTexture");
    pOtherProgram->setAttributeBuffer("vTexture", GL_FLOAT, offset, 2, 8*sizeof(float));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    floorBuf.release();
}
