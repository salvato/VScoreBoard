#pragma once

#include "particle.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>


class ParticleGenerator : protected QOpenGLFunctions_3_3_Core
{
public:
    ParticleGenerator(QOpenGLTexture* pTexture, uint amount);

public:
    void Update(float dt,
                uint newParticles,
                QVector3D offset=QVector3D(0.0f, 0.0f, 0.0f));
    void init(QVector3D origin);
    void draw(QOpenGLShaderProgram* pShader);

private:
    uint firstUnusedParticle();
    void respawnParticle(Particle* pParticle,
                         QVector3D offset=QVector3D(0.0f, 0.0f, 0.0f));

private:
    QOpenGLTexture* pTexture;
    QVector<Particle*> particles;
    QVector3D origin;
    const QVector3D gravity = QVector3D(0.0f, 9.81f, 0.0f);
    uint amount;
    float life;
    uint lastUsedParticle = 0;
};
