#include "playfield.h"
#include <QVector>


PlayField::PlayField(float xField, float zField) {
    initializeOpenGLFunctions();

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
}


void
PlayField::draw(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
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
