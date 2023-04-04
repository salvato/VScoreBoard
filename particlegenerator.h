#pragma once

#include "particle.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>


class ParticleGenerator : protected QOpenGLFunctions_3_3_Core
{
public:
    ParticleGenerator(QOpenGLTexture* pTexture,
                      uint amount);

public:
    void Update(float dt,
                Object &object,
                uint newParticles,
                QVector3D offset=QVector3D(0.0f, 0.0f, 0.0f));
    void draw(QOpenGLShaderProgram* pShader);

private:
    void init();
    uint firstUnusedParticle();
    void respawnParticle(Particle* pParticle,
                         Object object,
                         QVector3D offset=QVector3D(0.0f, 0.0f, 0.0f));

private:
    QVector<Particle*> particles;
    uint amount;
    QOpenGLTexture* pTexture;
    uint VAO;
};
