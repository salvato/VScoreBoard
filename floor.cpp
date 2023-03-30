#include "floor.h"
#include <QVector>


Floor::Floor(int xFloor, int zFloor) {
    initializeOpenGLFunctions();

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

}


void
Floor::draw(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
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
