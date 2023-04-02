#include "floor.h"

#include <QVector>
#include <QOpenglTexture>


Floor::Floor(QSizeF      _size,
             QVector3D   _position,
             QQuaternion _rotation,
             QVector3D   _scale,
             QVector3D   _speed)
    : Object(_position,
             _rotation,
             _scale,
             _speed)
{
    initializeOpenGLFunctions();

    float xFloor = _size.width();
    float zFloor = _size.height();

    QVector<float>  vertices;
    vertices <<
        // positions                 // normals               // texcoords
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
Floor::draw(QOpenGLShaderProgram* pProgram) {
    if(pTexture)
        pTexture->bind();
    pProgram->setUniformValue("model", modelMatrix());

    floorBuf.bind();

    // Offset for position
    quintptr offset = 0;
    int vertexLocation = pProgram->attributeLocation("vPosition");
    pProgram->enableAttributeArray(vertexLocation);
    pProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, 8*sizeof(float));

    // Offset for normal coordinate
    offset += 3*sizeof(float);
    int normalLocation = pProgram->attributeLocation("vNormal");
    pProgram->enableAttributeArray(normalLocation);
    pProgram->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, 8*sizeof(float));

    // Offset for texture coordinate
    offset += 3*sizeof(float);
    int texcoordLocation = pProgram->attributeLocation("vTexture");
    pProgram->enableAttributeArray(texcoordLocation);
    pProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, 8*sizeof(float));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    floorBuf.release();
}
