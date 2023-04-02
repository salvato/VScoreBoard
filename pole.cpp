#include "pole.h"

#include <QVector2D>
#include <QVector3D>

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
    QVector3D normals;
};


Pole::Pole(QSizeF      _size,
           QVector3D   _position,
           QQuaternion _rotation,
           QVector3D   _scale,
           QVector3D   _speed)
    : Object(_position,
             _rotation,
             _scale,
             _speed)
    , indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes the geometry and transfers it to VBOs
    initGeometry(_size.width(), _size.height());
}


Pole::~Pole() {
    arrayBuf.destroy();
    indexBuf.destroy();
}


void
Pole::initGeometry(float height, float diameter) {
    float r = 0.5*diameter;
    float h = 0.5*height;
    QVector3D normal0 = QVector3D( 0.0, 0.0, 1.0);
    QVector3D normal1 = QVector3D( 1.0, 0.0, 0.0);
    QVector3D normal2 = QVector3D( 0.0, 0.0,-1.0);
    QVector3D normal3 = QVector3D(-1.0, 0.0, 0.0);
    QVector3D normal4 = QVector3D( 0.0,-1.0, 0.0);
    QVector3D normal5 = QVector3D( 0.0, 1.0, 0.0);
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-r, -h,  r), QVector2D(0.0f, 0.0f), normal0},  // v0
        {QVector3D( r, -h,  r), QVector2D(1.0f, 0.0f), normal0}, // v1
        {QVector3D(-r,  h,  r), QVector2D(0.0f, 1.0f), normal0},  // v2
        {QVector3D( r,  h,  r), QVector2D(1.0f, 1.0f), normal0}, // v3

        // Vertex data for face 1
        {QVector3D( r, -h,  r), QVector2D(0.0f, 0.0f), normal1}, // v4
        {QVector3D( r, -h, -r), QVector2D(1.0f, 0.0f), normal1}, // v5
        {QVector3D( r,  h,  r), QVector2D(0.0f, 1.0f), normal1},  // v6
        {QVector3D( r,  h, -r), QVector2D(1.0f, 1.0f), normal1}, // v7

        // Vertex data for face 2
        {QVector3D( r, -h, -r), QVector2D(0.0f, 0.0f), normal2}, // v8
        {QVector3D(-r, -h, -r), QVector2D(1.0f, 0.0f), normal2},  // v9
        {QVector3D( r,  h, -r), QVector2D(0.0f, 1.0f), normal2}, // v10
        {QVector3D(-r,  h, -r), QVector2D(1.0f, 1.0f), normal2},  // v11

        // Vertex data for face 3
        {QVector3D(-r, -h, -r), QVector2D(0.0f, 0.0f), normal3}, // v12
        {QVector3D(-r, -h,  r), QVector2D(1.0f, 0.0f), normal3},  // v13
        {QVector3D(-r,  h, -r), QVector2D(0.0f, 1.0f), normal3}, // v14
        {QVector3D(-r,  h,  r), QVector2D(1.0f, 1.0f), normal3},  // v15

        // Vertex data for face 4
        {QVector3D(-r, -h, -r), QVector2D(0.0f, 0.0f), normal4}, // v16
        {QVector3D( r, -h, -r), QVector2D(1.0f, 0.0f), normal4}, // v17
        {QVector3D(-r, -h,  r), QVector2D(0.0f, 1.0f), normal4}, // v18
        {QVector3D( r, -h,  r), QVector2D(1.0f, 1.0f), normal4}, // v19

        // Vertex data for face 5
        {QVector3D(-r,  h,  r), QVector2D(0.0f, 0.0f), normal5}, // v20
        {QVector3D( r,  h,  r), QVector2D(1.0f, 0.0f), normal5}, // v21
        {QVector3D(-r,  h, -r), QVector2D(0.0f, 1.0f), normal5}, // v22
        {QVector3D( r,  h, -r), QVector2D(1.0f, 1.0f), normal5}  // v23
    };

    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, 24 * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 34 * sizeof(GLushort));
}


void
Pole::draw(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    pProgram->enableAttributeArray("vPosition");
    pProgram->setAttributeBuffer("vPosition", GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    pProgram->enableAttributeArray("vTexture");
    pProgram->setAttributeBuffer("vTexture", GL_FLOAT, offset, 2, sizeof(VertexData));

    offset += sizeof(QVector2D);
    pProgram->enableAttributeArray("vNormal");
    pProgram->setAttributeBuffer("vNormal", GL_FLOAT, offset, 3, sizeof(VertexData));

    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
}
