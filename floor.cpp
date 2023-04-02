#include "floor.h"
#include <QVector>


Floor::Floor(QSizeF      _size,
             QVector3D   _position,
             QQuaternion _rotation,
             QVector3D   _scale,
             QVector3D   _speed)
    : size(_size)
    , position(_position)
    , rotation(_rotation)
    , scale(_scale)
    , speed(_speed)
{
    initializeOpenGLFunctions();

    float xFloor = size.width();
    float zFloor = size.height();

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


QVector3D
Floor::getPos() {
    return position;
}


void
Floor::setPos(QVector3D newPos) {
    position = newPos;
}


QQuaternion
Floor::getRotation() {
    return rotation;
}


void
Floor::setRotation(QQuaternion newRotation) {
    rotation = newRotation;
}


QVector3D
Floor::getSpeed(){
    return speed;
}


void
Floor::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}


void
Floor::setScale(QVector3D newScale) {
    scale = newScale;
}


QVector3D
Floor::getScale() {
    return scale;
}


void
Floor::updateStatus(float deltaTime) {
    (void) deltaTime;
}


QMatrix4x4
Floor::modelMatrix() {
    QMatrix4x4 M;
    M.setToIdentity();
    M.translate(position);
    M.rotate(rotation);
    M.scale(scale);
    return M;
}
