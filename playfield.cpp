#include "playfield.h"
#include <QVector>


PlayField::PlayField(QSizeF      _size,
                     QVector3D   _position,
                     QQuaternion _rotation,
                     QVector3D   _speed)
    : size(_size)
    , position(_position)
    , rotation(_rotation)
    , speed(_speed)
{
    initializeOpenGLFunctions();
    float xField = size.width();
    float zField = size.height();

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


QVector3D
PlayField::getPos() {
    return position;
}


void
PlayField::setPos(QVector3D newPos) {
    position = newPos;
}


QQuaternion
PlayField::getRotation() {
    return rotation;
}


void
PlayField::setRotation(QQuaternion newRotation) {
    rotation = newRotation;
}


QVector3D
PlayField::getSpeed(){
    return speed;
}


void
PlayField::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}


void
PlayField::updateStatus(float deltaTime) {
    (void) deltaTime;
}


QMatrix4x4
PlayField::modelMatrix() {
    QMatrix4x4 M;
    M.setToIdentity();
    M.translate(position);
    M.rotate(rotation);
    return M;
}
