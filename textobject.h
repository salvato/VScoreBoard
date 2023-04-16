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
    TextObject(QOpenGLShaderProgram* _pProgram = nullptr,
               QOpenGLTexture*       _pTexture = nullptr,
               QVector3D             _position = QVector3D(0.0f, 0.0f, 0.0f),
               QQuaternion           _rotation = QQuaternion(),
               QVector3D             _scale    = QVector3D(1.0f, 1.0f, 1.0f),
               QVector3D             _speed    = QVector3D(0.0f, 0.0f, 0.0f));

public:
    void setText(QString _sText);
    void setColor(QColor _color=QColor(1.0f, 1.0f, 1.0f, 1.0f));
    void setHeight(ushort _height=48);
    void setDepth(float _depth=0.2f);
    bool GetRadius(float& radius) const;
    bool GetCenter(float& x, float& y, float& z) const;
    bool GetMin(float& x, float& y, float& z) const;
    bool GetMax(float& x, float& y, float& z) const;
    void updateStatus(float deltaTime) override;
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
