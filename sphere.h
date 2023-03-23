#pragma once


#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


class Sphere : protected QOpenGLFunctions
{
public:
    // construct a sphere centered at origin
    Sphere(float radius, int slices, int stacks);
    ~Sphere();
    // draw the sphere using provided shader program
    void draw(QOpenGLShaderProgram* pProgram);
    // Set Ambient and Diffuse color of Sphere
    inline void setColor(const QVector3D &color) {
      m_color = QVector4D(color, 1.);
    }
    inline void setSpecularColor(const QVector3D &color) {
      m_spec_color = QVector4D(color, 1.0f);
    }
    // Get current color of object
    inline QVector4D getAmbientAndDiffuseColor() const { return m_color; }
    inline QVector4D getSpecularColor() const { return m_spec_color; }

  private:
    // Generate Vertex Buffer Objects, but do not bind/allocate.
    // Return true if generate was a success, false otherwise
    bool initVBO();
    // setUp VAO once on firstDraw
    void setupVAO(QOpenGLShaderProgram* pProgram);

  protected:
    QOpenGLBuffer*            m_vbo;
    QOpenGLVertexArrayObject* m_vao;
    QVector4D m_color;
    QVector4D m_spec_color;
    bool m_firstDraw;
    float m_radius;
    int m_slices;
    int m_stacks;
    int m_stripsize; // num vertices per horizontal strip;
};
