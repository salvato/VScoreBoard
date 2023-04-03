#pragma once


#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


class Sphere : protected QOpenGLFunctions_3_3_Core
{
public:
    // construct a sphere centered at origin
    Sphere(float radius, int slices, int stacks);
    ~Sphere();
    // draw the sphere using provided shader program
    void draw(QOpenGLShaderProgram* pProgram);

  private:
    // Generate Vertex Buffer Objects, but do not bind/allocate.
    // Return true if generate was a success, false otherwise
    bool initVBO();
    // setUp VAO once on firstDraw
    void setupVAO(QOpenGLShaderProgram* pProgram);

  protected:
    QOpenGLBuffer*            m_vbo;
    QOpenGLVertexArrayObject* m_vao;
    bool m_firstDraw;
    float m_radius;
    int m_slices;
    int m_stacks;
    int m_stripsize; // num vertices per horizontal strip;
};
