#include "text3d.h"

#include <QImage>
#include <QOpenGLTexture>


struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
    QVector3D normals;
};



Text3D::Text3D(QString         _sText,
               QOpenGLTexture* _pTexture,
               QSizeF          _size,
               QVector3D       _position,
               QQuaternion     _rotation,
               QVector3D       _scale,
               QVector3D       _speed)
    : Object(_pTexture,
             _position,
             _rotation,
             _scale,
             _speed)
    , indexBuf(QOpenGLBuffer::IndexBuffer)
    , sText(_sText)
{
    (void)_size;
    initializeOpenGLFunctions();

    scale = QVector3D(1.0f, 0.1f, 1.0f);

    if(FT_Init_FreeType(&ft)) {
        qCritical() << "ERROR::FREETYPE: Could not init FreeType Library";
        exit(EXIT_FAILURE);
    }
    if(FT_New_Face(ft, "C:/Users/gabriele/Documents/qtprojects/VScoreBoard/arial.ttf", 0, &face)) {
        qCritical() << "ERROR::FREETYPE: Failed to load font";
        exit(EXIT_FAILURE);
    }
    if(FT_Set_Pixel_Sizes(face, 0, 48)) {
        qCritical() << "ERROR::FREETYPE: Failed to set font size";
        exit(EXIT_FAILURE);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    QOpenGLTexture* pTexture;
    QImage* pGlyph;
    for(uchar c=0; c<128; c++) {
        // load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            qCritical() << "ERROR::FREETYTPE: Failed to load Glyph";
            continue;
        }
        // generate texture
        if(face->glyph->bitmap.width*face->glyph->bitmap.rows != 0) {
            pGlyph = new QImage(face->glyph->bitmap.buffer,
                                face->glyph->bitmap.width,
                                face->glyph->bitmap.rows,
                                face->glyph->bitmap.width,
                                QImage::Format_Alpha8);
        }
        else {
            pGlyph = new QImage(1, 1, QImage::Format_Alpha8);
            pGlyph->fill(0);
        }
        pTexture = new QOpenGLTexture(*pGlyph);
        pTexture->setMagnificationFilter(QOpenGLTexture::Linear);
        pTexture->setMinificationFilter(QOpenGLTexture::Linear);
        pTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
        // now store character for later use
        Character character = {
            pTexture,
            QVector2D(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            QVector2D(face->glyph->bitmap_left,  face->glyph->bitmap_top),
            static_cast<uint>(face->glyph->advance.x)
        };
        Characters.insert(c, character);
    }
    pTexture->release();

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
//    vertices <<
//        // positions               // normals                // texcoords
//        -0.5f << 0.0f << 0.5f <<    0.0f <<-1.0f << 0.0f <<   0.0f << 1.0f <<
//         0.5f << 0.0f << 0.5f <<    0.0f <<-1.0f << 0.0f <<   1.0f << 1.0f <<
//        -0.5f << 0.0f <<-0.5f <<    0.0f <<-1.0f << 0.0f <<   0.0f << 0.0f <<

//         0.5f << 0.0f <<-0.5f <<    0.0f <<-1.0f << 0.0f <<   1.0f << 0.0f <<
//        -0.5f << 0.0f <<-0.5f <<    0.0f <<-1.0f << 0.0f <<   0.0f << 0.0f <<
//         0.5f << 0.0f << 0.5f <<    0.0f <<-1.0f << 0.0f <<   1.0f << 1.0f;

    arrayBuf.create();
    indexBuf.create();
    // Initializes the geometry and transfers it to VBOs
    initGeometry(2.0f, 0.1f, 2.0f);
}


Text3D::~Text3D() {
    arrayBuf.destroy();
    indexBuf.destroy();
}


void
Text3D::initGeometry(float height, float depth, float width) {
    float r = 0.5*width;
    float h = 0.5*height;
    float d = 0.5*depth;
    QVector3D normal0 = QVector3D( 0.0, 0.0, 1.0);
    QVector3D normal1 = QVector3D( 1.0, 0.0, 0.0);
    QVector3D normal2 = QVector3D( 0.0, 0.0,-1.0);
    QVector3D normal3 = QVector3D(-1.0, 0.0, 0.0);
    QVector3D normal4 = QVector3D( 0.0,-1.0, 0.0);
    QVector3D normal5 = QVector3D( 0.0, 1.0, 0.0);
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-r, -d,  h), QVector2D(0.0f, 0.0f), normal0},  // v0
        {QVector3D( r, -d,  h), QVector2D(0.0f, 0.0f), normal0},  // v1
        {QVector3D(-r,  d,  h), QVector2D(0.0f, 0.0f), normal0},  // v2
        {QVector3D( r,  d,  h), QVector2D(0.0f, 0.0f), normal0},  // v3

        // Vertex data for face 1
        {QVector3D( r, -d,  h), QVector2D(0.0f, 0.0f), normal1}, // v4
        {QVector3D( r, -d, -h), QVector2D(0.0f, 0.0f), normal1}, // v5
        {QVector3D( r,  d,  h), QVector2D(0.0f, 0.0f), normal1}, // v6
        {QVector3D( r,  d, -h), QVector2D(0.0f, 0.0f), normal1}, // v7

        // Vertex data for face 2
        {QVector3D( r, -d, -h), QVector2D(0.0f, 0.0f), normal2}, // v8
        {QVector3D(-r, -d, -h), QVector2D(0.0f, 0.0f), normal2}, // v9
        {QVector3D( r,  d, -h), QVector2D(0.0f, 0.0f), normal2}, // v10
        {QVector3D(-r,  d, -h), QVector2D(0.0f, 0.0f), normal2}, // v11

        // Vertex data for face 3
        {QVector3D(-r, -d, -h), QVector2D(0.0f, 0.0f), normal3}, // v12
        {QVector3D(-r, -d,  h), QVector2D(0.0f, 0.0f), normal3}, // v13
        {QVector3D(-r,  d, -h), QVector2D(0.0f, 0.0f), normal3}, // v14
        {QVector3D(-r,  d,  h), QVector2D(0.0f, 0.0f), normal3}, // v15

        // Vertex data for face 4
        {QVector3D(-r, -d, -h), QVector2D(0.0f, 0.0f), normal4}, // v16
        {QVector3D( r, -d, -h), QVector2D(1.0f, 0.0f), normal4}, // v17
        {QVector3D(-r, -d,  h), QVector2D(0.0f, 1.0f), normal4}, // v18
        {QVector3D( r, -d,  h), QVector2D(1.0f, 1.0f), normal4}, // v19

        // Vertex data for face 5
        {QVector3D(-r,  d,  h), QVector2D(0.0f, 0.0f), normal5}, // v20
        {QVector3D( r,  d,  h), QVector2D(1.0f, 0.0f), normal5}, // v21
        {QVector3D(-r,  d, -h), QVector2D(0.0f, 1.0f), normal5}, // v22
        {QVector3D( r,  d, -h), QVector2D(1.0f, 1.0f), normal5}  // v23
    };

    GLushort indices[] = {
        0,  1,  2,  3,  3,      // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
        4,  4,  5,  6,  7,  7,  // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
        8,  8,  9, 10,  11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
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
Text3D::draw(QOpenGLShaderProgram* pProgram) {
    pProgram->bind();
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    float x = position.x();
//    float z = position.z();

    QMatrix4x4 m = modelMatrix();
    for(int i=0; i<sText.count(); i++) {//  iterate through all characters
        Character ch = Characters[sText.at(i).toLatin1()];
//        float xpos = x + ch.Bearing.x() * scale.x();
//        float zpos = z - (ch.Size.y() - ch.Bearing.y()) * scale.z();
//        float w = ch.Size.x() * scale.x();
//        float h = ch.Size.y() * scale.z();
//        for(int i=0; i<6; i++) {
//            vertices[i*8]   = xpos;
//            vertices[i*8+2] = zpos;
//        }
//        vertices[2*8]   += w;
//        vertices[4*8]   += w;
//        vertices[5*8]   += w;
//        vertices[2]     += h;
//        vertices[3*8+2] += h;
//        vertices[5*8+2] += h;

        m.translate(1.0f, 0.0f, 0.0f);
        pProgram->setUniformValue("model", m);
        ch.pTexture->bind();
//        pTexture->bind();
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

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
//        x += (ch.Advance >> 6) * scale.x(); // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    arrayBuf.release();
    indexBuf.release();
}
