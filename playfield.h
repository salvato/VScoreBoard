#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


class PlayField : protected QOpenGLFunctions
{
public:
    PlayField();
    virtual ~PlayField();

public:
    void draw(QOpenGLShaderProgram* pProgram);
    inline void setColor(const QVector3D &color) {
      m_color = QVector4D(color, 1.);
    }
    inline void setSpecularColor(const QVector3D &color) {
      m_spec_color = QVector4D(color, 1.0f);
    }
    inline QVector4D getAmbientAndDiffuseColor() const { return m_color; }
    inline QVector4D getSpecularColor() const { return m_spec_color; }

private:
    void init();
    bool initVBO(); // Generate Vertex Buffer Objects, but do not bind/allocate.
    void setupVAO(QOpenGLShaderProgram* pProg); // setUp VAO once on firstDraw

private:
    QOpenGLBuffer*            pVbo; // vertexBuf;
    QOpenGLVertexArrayObject* pVao;

    QVector4D m_color;
    QVector4D m_spec_color;
    bool m_firstDraw;
    int nverts;
};
