#include "playfield.h"

#include <QVector2D>
#include <QVector3D>


PlayField::PlayField()
    : pVbo(nullptr)
    , indexBuf(QOpenGLBuffer::IndexBuffer)
    , m_color(0.0f, 0.0f, 1.0f, 1.0f)
    , m_spec_color(1.0f, 1.0f, 1.0f, 1.0f)
{
    initializeOpenGLFunctions();
    m_firstDraw = true;
    // Initializes Play Field and transfers it to VBOs
    init();
}


PlayField::~PlayField() {
    if(pVbo) {
        pVbo->release();
        delete pVbo;
        pVbo = nullptr;
    }
    if(pVao) {
        pVao->release();
        delete pVao;
        pVao = nullptr;
    }
    indexBuf.release();
}


void
PlayField::init() { // A simple cube at present !
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> tangents;
    QVector<QVector2D> texCoords;

    QVector3D normal;
    QVector3D tangent;
    // Vertex data for face 0 (z=1.0)
    normal = QVector3D(0.0, 0.0, 1.0);
    tangent= QVector3D(1.0, 1.0, 0.0).normalized();
    vertices.append(QVector3D(-1.0f, -1.0f,  1.0f)); // v0
    vertices.append(QVector3D( 1.0f, -1.0f,  1.0f)); // v1
    vertices.append(QVector3D(-1.0f,  1.0f,  1.0f)); // v2
    vertices.append(QVector3D( 1.0f,  1.0f,  1.0f)); // v3
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    texCoords.append(QVector2D(0.00f, 0.0f)); // v0
    texCoords.append(QVector2D(0.33f, 0.0f)); // v1
    texCoords.append(QVector2D(0.00f, 0.5f)); // v2
    texCoords.append(QVector2D(0.33f, 0.5f)); // v3

    // Vertex data for face 1 (x=1.0)
    normal = QVector3D(1.0, 0.0, 0.0);
    tangent= QVector3D(0.0, 1.0, 1.0).normalized();
    vertices.append(QVector3D( 1.0f, -1.0f,  1.0f));
    vertices.append(QVector3D( 1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D( 1.0f,  1.0f,  1.0f));
    vertices.append(QVector3D( 1.0f,  1.0f, -1.0f));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    texCoords.append(QVector2D(0.00f, 0.5f)); // v4
    texCoords.append(QVector2D(0.33f, 0.5f)); // v5
    texCoords.append(QVector2D(0.00f, 1.0f)); // v6
    texCoords.append(QVector2D(0.33f, 1.0f)); // v7

    // Vertex data for face 2 (z=-1.0)
    normal = QVector3D(0.0, 0.0, -1.0);
    tangent= QVector3D(1.0, 1.0, 0.0).normalized();
    vertices.append(QVector3D( 1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D(-1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D( 1.0f,  1.0f, -1.0f));
    vertices.append(QVector3D(-1.0f,  1.0f, -1.0f));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    texCoords.append(QVector2D(0.66f, 0.5f)); // v8
    texCoords.append(QVector2D(1.00f, 0.5f)); // v9
    texCoords.append(QVector2D(0.66f, 1.0f)); // v10
    texCoords.append(QVector2D(1.00f, 1.0f)); // v11

    // Vertex data for face 3 (x=-1)
    normal = QVector3D(-1.0, 0.0, 0.0);
    tangent= QVector3D( 0.0, 1.0, 1.0).normalized();
    vertices.append(QVector3D(-1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D(-1.0f, -1.0f,  1.0f));
    vertices.append(QVector3D(-1.0f,  1.0f, -1.0f));
    vertices.append(QVector3D(-1.0f,  1.0f,  1.0f));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    texCoords.append(QVector2D(0.66f, 0.0f)); // v12
    texCoords.append(QVector2D(1.00f, 0.0f)); // v13
    texCoords.append(QVector2D(0.66f, 0.5f)); // v14
    texCoords.append(QVector2D(1.00f, 0.5f)); // v15

    // Vertex data for face 4 (y=-1.0)
    normal = QVector3D(0.0,-1.0, 0.0);
    tangent= QVector3D(-1.0, 0.0, -1.0).normalized();
    vertices.append(QVector3D(-1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D( 1.0f, -1.0f, -1.0f));
    vertices.append(QVector3D(-1.0f, -1.0f,  1.0f));
    vertices.append(QVector3D( 1.0f, -1.0f,  1.0f));
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    normals.append(normal);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    tangents.append(tangent);
    texCoords.append(QVector2D(0.33f, 0.0f)); // v16
    texCoords.append(QVector2D(0.66f, 0.0f)); // v17
    texCoords.append(QVector2D(0.33f, 0.5f)); // v18
    texCoords.append(QVector2D(0.66f, 0.5f)); // v19

    // Vertex data for face 5 (y=1.0)
    normal = QVector3D(0.0, 1.0, 0.0);
    vertices.append(QVector3D(-1.0f,  1.0f,  1.0f));
    normals.append(normal);
    tangents.append(QVector3D(0.0f, 1.0f, 0.0f));
    texCoords.append(QVector2D(0.33f, 0.5f));        // v20
    vertices.append(QVector3D( 1.0f,  1.0f,  1.0f));
    normals.append(normal);
    tangents.append(QVector3D(0.0f, 1.0f, 0.0f));
    texCoords.append(QVector2D(0.66f, 0.5f));        // v21
    vertices.append(QVector3D(-1.0f,  1.0f, -1.0f));
    normals.append(normal);
    tangents.append(QVector3D(0.0f, 1.0f, 0.0f));
    texCoords.append(QVector2D(0.33f, 1.0f));        // v22
    vertices.append(QVector3D( 1.0f,  1.0f, -1.0f));
    normals.append(normal);
    tangents.append(QVector3D(0.0f, 1.0f, 0.0f));
    texCoords.append(QVector2D(0.66f, 1.0f));        // v23

    nverts = vertices.count();

    int VertexSize = nverts*sizeof(QVector3D);
    int TexSize    = nverts*sizeof(QVector2D);
    int NormalSize = VertexSize;
    int TangentSize= VertexSize;

    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

    nIndices = sizeof(indices)/sizeof(*indices);
    // Transfer vertex data to VBO
    pVbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(!pVbo->create()) exit(EXIT_FAILURE);
    pVbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    pVbo->bind();
    pVbo->allocate(VertexSize+TexSize+NormalSize+TangentSize);
    pVbo->write(0, vertices.constData(), VertexSize);
    pVbo->write(VertexSize, texCoords.constData(), TexSize);
    pVbo->write(VertexSize + TexSize, normals.constData(), NormalSize);
    pVbo->write(VertexSize + TexSize + NormalSize, tangents.constData(), TangentSize);
    pVbo->release();

    vertices.clear();
    texCoords.clear();
    normals.clear();
    tangents.clear();

    // Transfer index data to VBO 1
    indexBuf.create();
    indexBuf.bind();
    indexBuf.allocate(indices, nIndices * sizeof(GLushort));
}


void
PlayField::setupVAO(QOpenGLShaderProgram *prog) {
    pVao = new QOpenGLVertexArrayObject();
    if(!pVao->create()) exit(EXIT_FAILURE);
    pVao->bind();
    pVbo->bind();
    prog->enableAttributeArray("vPosition");
    prog->setAttributeBuffer("vPosition", GL_FLOAT, 0, 3, 0);
    prog->enableAttributeArray("vTexture");
    prog->setAttributeBuffer("vTexture", GL_FLOAT, nverts * sizeof(QVector3D), 2, 0);
    prog->enableAttributeArray("vNormal");
    prog->setAttributeBuffer("vNormal", GL_FLOAT,
                             nverts * (sizeof(QVector3D) + sizeof(QVector2D)), 3, 0);
    prog->enableAttributeArray("vTangent");
    prog->setAttributeBuffer("vTangent", GL_FLOAT,
                             nverts * (2 * sizeof(QVector3D) + sizeof(QVector2D)), 3, 0);
    pVao->release();
    pVbo->release();
}



void
PlayField::draw(QOpenGLShaderProgram* pProgram) {
    if (m_firstDraw) {
        setupVAO(pProgram);
        m_firstDraw = false;
    }
    pVao->bind();
    indexBuf.bind();
    QVector4D clr =  m_color;
    pProgram->setUniformValue("vColor", clr);
    pProgram->setUniformValue("vSColor", m_spec_color);
    glDrawElements(GL_TRIANGLE_STRIP, nIndices, GL_UNSIGNED_SHORT, nullptr);
    pVao->release();
}
