#include "whiteline.h"

WhiteLine::WhiteLine(QSizeF      _size,
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

    float xDim = size.width();
    float zDim = size.height();

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


QVector3D
WhiteLine::getPos() {
    return position;
}


void
WhiteLine::setPos(QVector3D newPos) {
    position = newPos;
}


QQuaternion
WhiteLine::getRotation() {
    return rotation;
}


void
WhiteLine::setRotation(QQuaternion newRotation) {
    rotation = newRotation;
}


QVector3D
WhiteLine::getSpeed(){
    return speed;
}


void
WhiteLine::setSpeed(QVector3D newSpeed) {
    speed = newSpeed;
}


void
WhiteLine::setScale(QVector3D newScale) {
    scale = newScale;
}


QVector3D
WhiteLine::getScale() {
    return scale;
}


void
WhiteLine::updateStatus(float deltaTime) {
    (void) deltaTime;
}


QMatrix4x4
WhiteLine::modelMatrix() {
    QMatrix4x4 M;
    M.setToIdentity();
    M.translate(position);
    M.rotate(rotation);
    M.scale(scale);
    return M;
}
