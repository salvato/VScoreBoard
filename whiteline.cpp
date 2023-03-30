#include "whiteline.h"

WhiteLine::WhiteLine(float xDim, float zDim) {
    initializeOpenGLFunctions();

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
}


void
WhiteLine::draw(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
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
