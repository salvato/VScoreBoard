#pragma once

#include "object.h"
#include "ft2build.h"
#include <freetype/freetype.h>

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class Text3D : public Object, protected QOpenGLFunctions_3_3_Core
{
public:
    Text3D(QString         _sText,
           QOpenGLTexture* _pTexture = nullptr,
           QSizeF          _size     = QSizeF(1.0f, 1.0f),
           QVector3D       _position = QVector3D(0.0f, 0.0f, 0.0f),
           QQuaternion     _rotation = QQuaternion(),
           QVector3D       _scale    = QVector3D(1.0f, 1.0f, 1.0f),
           QVector3D       _speed    = QVector3D(0.0f, 0.0f, 0.0f));
    ~Text3D();

public:
    void draw(QOpenGLShaderProgram* pProgram) override;

protected:
    void initGeometry(float height, float depth, float width);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QString sText;
    FT_Library ft;
    FT_Face face;
    struct Character {
        QOpenGLTexture* pTexture; // ID handle of the glyph texture
        QVector2D       Size;     // Size of glyph
        QVector2D       Bearing;  // Offset from baseline to left/top of glyph
        uint            Advance;  // Offset to advance to next glyph
    };
    QMap<char, Character> Characters;
    QVector<float>  vertices;
};
