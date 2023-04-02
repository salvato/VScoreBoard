#include "playfield.h"

#include <QVector>
#include <QOpenglTexture>


PlayField::PlayField(QSizeF      _size,
                     QVector3D   _position,
                     QQuaternion _rotation, QVector3D _scale,
                     QVector3D   _speed)
    : Object(_position,
             _rotation,
             _scale,
             _speed)
{
    initializeOpenGLFunctions();

    float xField = _size.width();
    float zField = _size.height();

    QVector<float> vertices;
    vertices <<
    // positions                   // normals              // texcoords
    -xField << 0.0f <<  zField <<  0.0f << 1.0f << 0.0f <<   0.0f << zField <<
     xField << 0.0f <<  zField <<  0.0f << 1.0f << 0.0f << xField << zField <<
    -xField << 0.0f << -zField <<  0.0f << 1.0f << 0.0f <<   0.0f <<   0.0f <<

     xField << 0.0f << -zField <<  0.0f << 1.0f << 0.0f << xField <<   0.0f <<
    -xField << 0.0f << -zField <<  0.0f << 1.0f << 0.0f <<   0.0f <<   0.0f <<
     xField << 0.0f <<  zField <<  0.0f << 1.0f << 0.0f << xField << zField ;

    // Transfer vertex data to VBO
    fieldBuf.create();
    fieldBuf.bind();
    fieldBuf.allocate(vertices.data(), int(vertices.count()*sizeof(float)));
    fieldBuf.release();
}


PlayField::~PlayField() {
    fieldBuf.destroy();
}


void
PlayField::draw(QOpenGLShaderProgram* pProgram) {
    if(pTexture)
        pTexture->bind();
    pProgram->setUniformValue("model", modelMatrix());

    fieldBuf.bind();

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
    fieldBuf.release();
}

