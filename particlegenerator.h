#pragma once

#include "particle.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>


// ParticleGenerator acts as a container for rendering a large number of
// particles by repeatedly spawning and updating particles and killing
// them after a given amount of time.
class ParticleGenerator : protected QOpenGLFunctions_3_3_Core
{
public:
    // constructor
    ParticleGenerator(QOpenGLShaderProgram* pShader, QOpenGLTexture* pTexture, uint amount);
    // update all particles
    void Update(float dt, Object &object, unsigned int newParticles, QVector3D offset = QVector3D(0.0f, 0.0f, 0.0f));
    // render all particles
    void Draw();

private:
    // state
    QVector<Particle*> particles;
    uint amount;
    // render state
    QOpenGLShaderProgram* pShader;
    QOpenGLTexture* pTexture;
    uint VAO;
    // initializes buffer and vertex attributes
    void init();
    // returns the first Particle index that's currently unused
    // e.g. Life <= 0.0f or 0 if no particle is currently inactive
    uint firstUnusedParticle();
    // respawns particle
    void respawnParticle(Particle* pParticle, Object object, QVector3D offset = QVector3D(0.0f, 0.0f, 0.0f));
};
