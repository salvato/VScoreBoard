/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "cube.h"

#include <QVector2D>
#include <QVector3D>

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
    QVector3D normals;
};


Cube::Cube(float height, float diameter)
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes the geometry and transfers it to VBOs
    initGeometry(height, diameter);
}


Cube::~Cube() {
    arrayBuf.destroy();
    indexBuf.destroy();
}


void
Cube::initGeometry(float height, float diameter) {
    float ray = 0.5*diameter;
    QVector3D normal0 = QVector3D( 0.0, 0.0, 1.0);
    QVector3D normal1 = QVector3D( 1.0, 0.0, 0.0);
    QVector3D normal2 = QVector3D( 0.0, 0.0,-1.0);
    QVector3D normal3 = QVector3D(-1.0, 0.0, 0.0);
    QVector3D normal4 = QVector3D( 0.0,-1.0, 0.0);
    QVector3D normal5 = QVector3D( 0.0, 1.0, 0.0);
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-ray, -height,  ray), QVector2D(0.0f, 0.0f), normal0},  // v0
        {QVector3D( ray, -height,  ray), QVector2D(1.0f, 0.0f), normal0}, // v1
        {QVector3D(-ray,  height,  ray), QVector2D(0.0f, 1.0f), normal0},  // v2
        {QVector3D( ray,  height,  ray), QVector2D(1.0f, 1.0f), normal0}, // v3

        // Vertex data for face 1
        {QVector3D( ray, -height,  ray), QVector2D(0.0f, 0.0f), normal1}, // v4
        {QVector3D( ray, -height, -ray), QVector2D(1.0f, 0.0f), normal1}, // v5
        {QVector3D( ray,  height,  ray), QVector2D(0.0f, 1.0f), normal1},  // v6
        {QVector3D( ray,  height, -ray), QVector2D(1.0f, 1.0f), normal1}, // v7

        // Vertex data for face 2
        {QVector3D( ray, -height, -ray), QVector2D(0.0f, 0.0f), normal2}, // v8
        {QVector3D(-ray, -height, -ray), QVector2D(1.0f, 0.0f), normal2},  // v9
        {QVector3D( ray,  height, -ray), QVector2D(0.0f, 1.0f), normal2}, // v10
        {QVector3D(-ray,  height, -ray), QVector2D(1.0f, 1.0f), normal2},  // v11

        // Vertex data for face 3
        {QVector3D(-ray, -height, -ray), QVector2D(0.0f, 0.0f), normal3}, // v12
        {QVector3D(-ray, -height,  ray), QVector2D(1.0f, 0.0f), normal3},  // v13
        {QVector3D(-ray,  height, -ray), QVector2D(0.0f, 1.0f), normal3}, // v14
        {QVector3D(-ray,  height,  ray), QVector2D(1.0f, 1.0f), normal3},  // v15

        // Vertex data for face 4
        {QVector3D(-ray, -height, -ray), QVector2D(0.0f, 0.0f), normal4}, // v16
        {QVector3D( ray, -height, -ray), QVector2D(1.0f, 0.0f), normal4}, // v17
        {QVector3D(-ray, -height,  ray), QVector2D(0.0f, 1.0f), normal4}, // v18
        {QVector3D( ray, -height,  ray), QVector2D(1.0f, 1.0f), normal4}, // v19

        // Vertex data for face 5
        {QVector3D(-ray,  height,  ray), QVector2D(0.0f, 0.0f), normal5}, // v20
        {QVector3D( ray,  height,  ray), QVector2D(1.0f, 0.0f), normal5}, // v21
        {QVector3D(-ray,  height, -ray), QVector2D(0.0f, 1.0f), normal5}, // v22
        {QVector3D( ray,  height, -ray), QVector2D(1.0f, 1.0f), normal5}  // v23
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
Cube::draw(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = pProgram->attributeLocation("vPosition");
    pProgram->enableAttributeArray(vertexLocation);
    pProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = pProgram->attributeLocation("vTexture");
    pProgram->enableAttributeArray(texcoordLocation);
    pProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector2D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int normalLocation = pProgram->attributeLocation("vNormal");
    pProgram->enableAttributeArray(normalLocation);
    pProgram->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
}
