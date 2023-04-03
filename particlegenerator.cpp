
#include "particlegenerator.h"


ParticleGenerator::ParticleGenerator(QOpenGLShaderProgram* pShader, QOpenGLTexture* pTexture, uint amount)
    : amount(amount)
    , pShader(pShader)
    , pTexture(pTexture)
{
    init();
}


void
ParticleGenerator::Update(float dt, Object& object, unsigned int newParticles, QVector3D offset) {
    // add new particles
    for (unsigned int i = 0; i < newParticles; ++i) {
        int unusedParticle = firstUnusedParticle();
        respawnParticle(particles[unusedParticle], object, offset);
    }
    // update all particles
    for (unsigned int i = 0; i < amount; ++i) {
        ;
        particles.at(i)->life -= dt; // reduce life
        if (particles.at(i)->life > 0.0f) {	// particle is alive, thus update
            particles.at(i)->setPos(particles.at(i)->getPos() - particles.at(i)->getSpeed() * dt);
            particles.at(i)->color.setW(particles.at(i)->color.w() - dt*2.5f);
        }
    }
}


// render all particles
void
ParticleGenerator::Draw() {
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    pShader->bind();
    for(int i=0; i< particles.count(); i++) {
        if(particles.at(i)->life > 0.0f) {
            pShader->setUniformValue("offset", particles.at(i)->getPos());
            pShader->setUniformValue("color", particles.at(i)->color);
            pTexture->bind();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void
ParticleGenerator::init() {
    // set up mesh and attribute properties
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // create amount default particle instances
    for(uint i=0; i<amount; ++i)
        particles.push_back(new Particle());
}


// stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
unsigned int
ParticleGenerator::firstUnusedParticle() {
    // first search from last used particle, this will usually return almost instantly
    for(uint i=lastUsedParticle; i<amount; ++i) {
        if(particles.at(i)->life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for(uint i=0; i<lastUsedParticle; ++i){
        if(particles.at(i)->life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}


void
ParticleGenerator::respawnParticle(Particle *pParticle, Object object, QVector3D offset) {
    QVector3D random = QVector3D(((rand() % 100) - 50) / 10.0f,
                                 ((rand() % 100) - 50) / 10.0f,
                                 ((rand() % 100) - 50) / 10.0f);
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    pParticle->setPos(object.getPos() + random + offset);
    pParticle->color = QVector4D(rColor, rColor, rColor, 1.0f);
    pParticle->life = 1.0f;
    pParticle->setSpeed(object.getSpeed() * 0.1f);
}

