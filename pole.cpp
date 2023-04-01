#include "pole.h"

Pole::Pole(float height, float diameter) {
    float ray = 0.5f*diameter;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector2D> texCoords;

    QVector3D normal;

    // Vertex data for face 0 (z=1.0)
    normal = QVector3D(0.0, 0.0, 1.0);
    vertices.append(QVector3D(-ray, -height,  ray)); // v0
    vertices.append(QVector3D( ray, -height,  ray)); // v1
    vertices.append(QVector3D(-ray,  height,  ray)); // v2
    vertices.append(QVector3D( ray,  height,  ray)); // v3
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    // Vertex data for face 1 (x=1.0)
    normal = QVector3D(1.0, 0.0, 0.0);
    vertices.append(QVector3D( ray, -height,  ray));
    vertices.append(QVector3D( ray, -height, -ray));
    vertices.append(QVector3D( ray,  height,  ray));
    vertices.append(QVector3D( ray,  height, -ray));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    // Vertex data for face 2 (z=-1.0)
    normal = QVector3D(0.0, 0.0, -1.0);
    vertices.append(QVector3D( ray, -height, -ray));
    vertices.append(QVector3D(-ray, -height, -ray));
    vertices.append(QVector3D( ray,  height, -ray));
    vertices.append(QVector3D(-ray,  height, -ray));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    // Vertex data for face 3 (x=-1)
    normal = QVector3D(-1.0, 0.0, 0.0);
    vertices.append(QVector3D(-ray, -height, -ray));
    vertices.append(QVector3D(-ray, -height,  ray));
    vertices.append(QVector3D(-ray,  height, -ray));
    vertices.append(QVector3D(-ray,  height,  ray));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    // Vertex data for face 4 (y=-1.0)
    normal = QVector3D(0.0,-1.0, 0.0);
    vertices.append(QVector3D(-ray, -height, -ray));
    vertices.append(QVector3D( ray, -height, -ray));
    vertices.append(QVector3D(-ray, -height,  ray));
    vertices.append(QVector3D( ray, -height,  ray));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    // Vertex data for face 5 (y=1.0)
    normal = QVector3D(0.0, 1.0, 0.0);
    vertices.append(QVector3D(-ray,  height,  ray));
    vertices.append(QVector3D( ray,  height,  ray));
    vertices.append(QVector3D(-ray,  height, -ray));
    vertices.append(QVector3D( ray,  height, -ray));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    texCoords.append(QVector2D(0.0f, 0.0f)); // v0
    texCoords.append(QVector2D(1.0f, 0.0f)); // v1
    texCoords.append(QVector2D(0.0f, 1.0f)); // v2
    texCoords.append(QVector2D(1.0f, 1.0f)); // v3

    nverts = vertices.count();

    int VertexSize = nverts*sizeof(QVector3D);
    int TexSize    = nverts*sizeof(QVector2D);
    int NormalSize = VertexSize;

    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

    int nIndices = sizeof(indices)/sizeof(*indices);
    // Transfer vertex data to VBO
    pPoleBuf = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(!pPoleBuf->create()) {
        qCritical() << __FUNCTION__ << __LINE__ << "Error! Exiting";
        exit(EXIT_FAILURE);
    }
    pPoleBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
    pPoleBuf->bind();
    pPoleBuf->allocate(VertexSize+TexSize+NormalSize);
    pPoleBuf->write(0, vertices.constData(), VertexSize);
    pPoleBuf->write(VertexSize, texCoords.constData(), TexSize);
    pPoleBuf->write(VertexSize + TexSize, normals.constData(), NormalSize);
    pPoleBuf->release();

    vertices.clear();
    texCoords.clear();
    normals.clear();

    indexBuf.create();
    indexBuf.bind();
    indexBuf.allocate(indices, nIndices * sizeof(GLushort));
}


Pole::~Pole() {
}


void
Pole::draw(QOpenGLShaderProgram* pProgram) {
    pPoleBuf->bind();
    indexBuf.bind();

    quintptr offset = 0;
    pProgram->enableAttributeArray("vPosition");
    pProgram->setAttributeBuffer("vPosition", GL_FLOAT, offset, 3, 0);

    offset += nverts * sizeof(QVector3D);
    pProgram->enableAttributeArray("vTexture");
    pProgram->setAttributeBuffer("vTexture", GL_FLOAT, offset, 2, 0);

    offset += nverts * sizeof(QVector2D);
    pProgram->enableAttributeArray("vNormal");
    pProgram->setAttributeBuffer("vNormal", GL_FLOAT, offset, 3, 0);

//    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
}
