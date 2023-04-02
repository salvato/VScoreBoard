#include "whiteline.h"

#include <QOpenglTexture>

WhiteLine::WhiteLine(QSizeF      _size,
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

    float xDim = _size.width();
    float zDim = _size.height();

    QVector<float> vertices;
    vertices <<
    // positions               // normals              // texcoords
    -xDim << 0.0f <<  zDim <<  0.0f << 1.0f << 0.0f << 0.0f << zDim <<
     xDim << 0.0f <<  zDim <<  0.0f << 1.0f << 0.0f << xDim << zDim <<
    -xDim << 0.0f << -zDim <<  0.0f << 1.0f << 0.0f << 0.0f << 0.0f <<

     xDim << 0.0f << -zDim <<  0.0f << 1.0f << 0.0f << xDim << 0.0f <<
    -xDim << 0.0f << -zDim <<  0.0f << 1.0f << 0.0f << 0.0f << 0.0f <<
     xDim << 0.0f <<  zDim <<  0.0f << 1.0f << 0.0f << xDim << zDim ;

    // Transfer vertex data to VBO
    lineBuf.create();
    lineBuf.bind();
    lineBuf.allocate(vertices.data(), int(vertices.count()*sizeof(float)));
    lineBuf.release();
}


WhiteLine::~WhiteLine() {
    lineBuf.destroy();
}


void
WhiteLine::draw(QOpenGLShaderProgram* pProgram) {
    if(pTexture)
        pTexture->bind();
    pProgram->setUniformValue("model", modelMatrix());

    lineBuf.bind();

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
    lineBuf.release();
}
