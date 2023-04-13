#include "text3d.h"

#include <QImage>
#include <QOpenGLTexture>


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
             _speed),
    sText(_sText)
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
    vertices <<
        // positions               // normals                // texcoords
       -0.5f << 0.0f << 0.5f <<    0.0f << 1.0f << 0.0f <<   0.0f << 0.0f <<
        0.5f << 0.0f << 0.5f <<    0.0f << 1.0f << 0.0f <<   1.0f << 0.0f <<
       -0.5f << 0.0f <<-0.5f <<    0.0f << 1.0f << 0.0f <<   0.0f << 1.0f <<

        0.5f << 0.0f <<-0.5f <<    0.0f << 1.0f << 0.0f <<   1.0f << 1.0f <<
       -0.5f << 0.0f <<-0.5f <<    0.0f << 1.0f << 0.0f <<   0.0f << 1.0f <<
        0.5f << 0.0f << 0.5f <<    0.0f << 1.0f << 0.0f <<   1.0f << 0.0f;

    charBuf.create();
    charBuf.bind();
    charBuf.allocate(0, int(vertices.count()*sizeof(float)));
    charBuf.release();
}


Text3D::~Text3D() {
    charBuf.destroy();
}


void
Text3D::draw(QOpenGLShaderProgram* pProgram) {
    pProgram->bind();
    pProgram->setUniformValue("model", modelMatrix());
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float x = position.x();
    float z = position.z();

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

        ch.pTexture->bind();
        charBuf.bind();
        charBuf.write(0, vertices.constData(), int(vertices.count()*sizeof(float)));

        // Offset for position
        quintptr offset = 0;
        pProgram->enableAttributeArray("vPosition");
        pProgram->setAttributeBuffer("vPosition", GL_FLOAT, offset, 3, 8*sizeof(float));

        // Offset for normal coordinate
        offset += 3*sizeof(float);
        pProgram->enableAttributeArray("vNormal");
        pProgram->setAttributeBuffer("vNormal", GL_FLOAT, offset, 3, 8*sizeof(float));

        // Offset for texture coordinate
        offset += 3*sizeof(float);
        pProgram->enableAttributeArray("vTexture");
        pProgram->setAttributeBuffer("vTexture", GL_FLOAT, offset, 2, 8*sizeof(float));

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale.x(); // bitshift by 6 to get value in pixels (2^6 = 64)
        charBuf.release();
    }
}
