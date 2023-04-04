
#include "particlegenerator.h"

#define rnd (rand()/double(RAND_MAX)) // [0.0 - 1.0]

ParticleGenerator::ParticleGenerator(QOpenGLTexture* pTexture, uint amount)
    : pTexture(pTexture)
    , amount(amount)
    , life(3.0f)
{
    initializeOpenGLFunctions();
}


/*
    Choose three points u, v, w ∈ [0,1] uniformly at random.
    A uniform, random quaternion is given by the simple expression:

    q = (sqrt(1-u)*sin(2πv), sqrt(1-u)*cos(2πv), sqrt(u)*sin(2πw), sqrt(u)*cos(2πw))

    oppure:

    QQuaternion random_quaternion() {
        double x,y,z, u,v,w, s;
        do { x = random(-1,1); y = random(-1,1); z = x*x + y*y; } while (z > 1);
        do { u = random(-1,1); v = random(-1,1); w = u*u + v*v; } while (w > 1);
        s = sqrt((1-z) / w);
        return QQuaternion(x, y, s*u, s*v);
    }
*/


void
ParticleGenerator::init(QVector3D _origin) {
    particles.clear();
    origin = _origin;
    QQuaternion q;
    for(uint i=0; i<amount; ++i) {
        QVector3D random = QVector3D(((rand() % 100) - 50) / 100.0f,
                                     ((rand() % 100) - 50) / 100.0f,
                                     ((rand() % 100) - 50) / 100.0f);
        QVector3D velocity = QVector3D(((rand() % 100) - 50) / 20.0f,
                                        (rand() % 100)       /  7.0f,
                                       ((rand() % 100) - 50) / 20.0f);
        float u = rnd;
        float v = rnd;
        float w = rnd;
        q = QQuaternion(sqrt(1.0-u) * sin(2.0*M_PI*v),
                        sqrt(1.0-u) * cos(2.0*M_PI*v),
                        sqrt(u)     * sin(2.0*M_PI*w),
                        sqrt(u)     *  cos(2.0*M_PI*w));

        particles.push_back(new Particle(QVector4D(1.0f, 1.0f, 1.0f, 1.0f), // color
                                         life,                              // life
                                         QSizeF(0.07f, 0.07f),              // size
                                         pTexture,                          // texture
                                         origin + random,                   // position
                                         q,                                 // rotation
                                         QVector3D(1.0f, 1.0f, 1.0f),       // scale
                                         velocity)                          // speed
                            );
    }
}


void
ParticleGenerator::Update(float dt, uint newParticles, QVector3D offset) {
    // add new particles
    for(uint i=0; i<newParticles; ++i) {
        int unusedParticle = firstUnusedParticle();
        respawnParticle(particles[unusedParticle], offset);
    }
    // update all particles
    for(uint i=0; i<amount; ++i) {
        Particle* pParticle = particles.at(i);
        pParticle->life -= dt; // reduce life
        if (pParticle->life > 0.0f) {	// particle is alive, thus update
            pParticle->setPos(pParticle->getPos() +
                              pParticle->getSpeed() * dt);
            if(pParticle->getPos().y() < 0.0f)
                pParticle->life = 0.0f;
            pParticle->color.setW(pParticle->color.w() - dt*2.5f);
            pParticle->setSpeed(pParticle->getSpeed()-gravity*dt);
        }
    }
}


// render all particles
void
ParticleGenerator::draw(QOpenGLShaderProgram* pShader) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    pShader->bind();
    for(int i=0; i< particles.count(); i++) {
        if(particles.at(i)->life > 0.0f) {
            particles.at(i)->draw(pShader);
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


uint
ParticleGenerator::firstUnusedParticle() {
    for(uint i=lastUsedParticle; i<amount; ++i) {
        if(particles.at(i)->life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    for(uint i=0; i<lastUsedParticle; ++i){
        if(particles.at(i)->life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    lastUsedParticle = 0;
    return 0;
}


void
ParticleGenerator::respawnParticle(Particle *pParticle, QVector3D offset) {
    QVector3D random = QVector3D(((rand() % 100) - 50) / 100.0f,
                                 ((rand() % 100) - 50) / 100.0f,
                                 ((rand() % 100) - 50) / 100.0f);
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    pParticle->setPos(origin + random + offset);
    pParticle->color = QVector4D(rColor, rColor, rColor, 1.0f);
    pParticle->life = life;
    QVector3D velocity = QVector3D(((rand() % 100) - 50) / 20.0f,
                                   (rand() % 100)        /  7.0f,
                                   ((rand() % 100) - 50) / 20.0f);

    pParticle->setSpeed(velocity);
}

