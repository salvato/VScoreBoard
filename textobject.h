#pragma once

#include "object.h"
#include "model3d.h"
#include "model3dex.h"

#include <QOpenGLBuffer>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QOpenGLFunctions_4_4_Core>



class TextObject : public Object, protected QOpenGLFunctions_4_4_Core
{
public:
    TextObject();

public:
    void setText(QString _sText,
                 QColor _color=QColor(1.0f, 1.0f, 1.0f, 1.0f),
                 ushort _height=48,
                 float _depth=0.2f);
    void draw() override;
    void draw(QOpenGLShaderProgram* pOtherProgram) override;

protected:
    void createTextModel();
    float* generateVertexBuffer(const Model3D& model);
    void generateNormalsBuffer(const Model3D& model, float* coords);
    void create2DImage(const QString& sText, const QFont& font, ushort height);

    QOpenGLBuffer vertexTextBuf;
    QOpenGLBuffer normalTextBuf;
    Model3DEx     textModel;
    QImage*       pTextImage = nullptr;

private:
    QString sText;
    QColor color;
    ushort height;
    float depth;
    QFont font;
};
