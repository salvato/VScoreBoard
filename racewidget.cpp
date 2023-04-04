/*
 *
Copyright (C) 2023  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "racewidget.h"
#include "floor.h"
#include "playfield.h"
#include "whiteline.h"
#include "pole.h"
#include "avatar.h"
#include "particlegenerator.h"
#include "resourcemanager.h"


#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QIcon>
#include <QTime>


//#define SHOW_DEPTH


RaceWidget::RaceWidget()
    : QOpenGLWidget()
    , bRacing(false)
    , bFireWorks(false)

{
    QList<QScreen*> screens = QApplication::screens();
    QRect screenres = screens.at(0)->geometry();
    if(screens.count() > 1) {
        screenres = screens.at(1)->geometry();
        QPoint point = QPoint(screenres.x(), screenres.y());
        move(point);
    }

    setWindowIcon(QIcon(":/buttonIcons/plot.png"));
    sTeamName[0] = "Locali";
    sTeamName[1] = "Ospiti";

    ballRadius = 0.1066f * 4.0f; // 4 times bigger than real

    diffuseColor  = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

    cameraPosition = QVector4D(0.0f, 10.0f, 10.0f, 1.0f);
//    cameraPosition = QVector4D(-xField, 5.0f, 0.0f, 1.0f);
    cameraViewMatrix.lookAt(cameraPosition.toVector3D(),  // Eye
                            QVector3D(0.0f, 0.0f, 0.0f),  // Center
                            QVector3D(0.0f, 1.0f, 0.0f)); // Up

    lightPosition = QVector4D(-2.0f, 4.0f, -1.0f, 1.0f);

    float extension = std::max(xField, zField)*1.5;
    near_plane = -1.0f;
    far_plane  = extension;
    lightProjectionMatrix.setToIdentity();
    lightProjectionMatrix.ortho(-extension,
                                 extension,
                                -extension,
                                 extension,
                                 near_plane,
                                 far_plane);
    lightViewMatrix.lookAt(lightPosition.toVector3D(),     // Eye
                           QVector3D( 0.0f, 0.0f,  0.0f),  // Center
                           QVector3D( 0.0f, 1.0f,  0.0f)); // Up
    lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

    resetAll();
    scanTime = 10.0; // Tempo in secondi per l'intera "Corsa"
    speed = 2.0f*xField/scanTime;
    connect(&fireworksTimer, SIGNAL(timeout()),
             this, SLOT(onStopFireworks()));
    connect(&closeTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToClose()));
}


RaceWidget::~RaceWidget() {
    makeCurrent();
    doneCurrent();
}


void
RaceWidget::closeEvent(QCloseEvent* event) {
    timer.stop();
    makeCurrent();
    doneCurrent();
    emit raceDone();
    event->accept();
}


void
RaceWidget::updateLabel(int iTeam, QString sLabel) {
    if((iTeam < 0) || (iTeam > 1)) return;
    sTeamName[iTeam] = sLabel;
    update();
}


void
RaceWidget::updateScore(int team0Score, int team1Score, int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    score[iSet].append(QVector2D(team0Score, team1Score));
    maxScore[iSet] = std::max(team0Score, team1Score);
}


void
RaceWidget::resetScore(int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    score[iSet].clear();
    score[iSet].append(QVector2D(0, 0));
    maxScore[iSet] = 0;
}


void
RaceWidget::resetAll() {
    for(int i=0; i<5; i++) {
        score[i].clear();
        score[i].append(QVector2D(0, 0));
        maxScore[i] = 0;
    }
}


void
RaceWidget::resizeGL(int w, int h) {
    cameraProjectionMatrix.setToIdentity();
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01f;
    const qreal zFar  = 30.0f;
    const qreal fov   = 50.0;
    cameraProjectionMatrix.perspective(fov, aspect, zNear, zFar);
}


void
RaceWidget::initGameObjects() {
    gameObjects.clear();
    pFloor       = new Floor(QSizeF(50.0f, 50.0f),
                       ResourceManager::GetTexture("floor"));
    gameObjects.append(pFloor);

    // Slightltly higer than Floor
    pPlayField   = new PlayField(QSizeF(xField, zField),
                               ResourceManager::GetTexture("field"),
                               QVector3D(0.0f, 0.01f, 0.0f));
    gameObjects.append(pPlayField);

    // Slightltly higer than PlayField
    pLeftLine    = new WhiteLine(QSizeF(0.05f, zField),
                              ResourceManager::GetTexture("line"),
                              QVector3D(-xField+0.05f, 0.02f, 0.0f));
    gameObjects.append(pLeftLine);

    pLeft3mLine  = new WhiteLine(QSizeF(0.05f, zField),
                                ResourceManager::GetTexture("line"),
                                QVector3D(-3.0f+0.05f, 0.02f, 0.0f));
    gameObjects.append(pLeft3mLine);

    pCentralLine = new WhiteLine(QSizeF(0.05f, zField),
                                 ResourceManager::GetTexture("line"),
                                 QVector3D(0.0f, 0.02f, 0.0f));
    gameObjects.append(pCentralLine);
    pRight3mLine = new WhiteLine(QSizeF(0.05f, zField),
                                 ResourceManager::GetTexture("line"),
                                 QVector3D(3.0f-0.05f, 0.02f, 0.0f));
    gameObjects.append(pRight3mLine);
    pRightLine   = new WhiteLine(QSizeF(0.05f, zField),
                               ResourceManager::GetTexture("line"),
                               QVector3D(xField-0.05f, 0.02f, 0.0f));
    gameObjects.append(pRightLine);
    pBottomLine  = new WhiteLine(QSizeF(xField, 0.05f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 0.02f,  zField-0.05f));
    gameObjects.append(pBottomLine);
    pTopLine     = new WhiteLine(QSizeF(xField, 0.05f),
                             ResourceManager::GetTexture("line"),
                             QVector3D(0.0f, 0.02f, -zField+0.05f));
    gameObjects.append(pTopLine);
    pBottomPole  = new Pole(QSizeF(2.43f, 0.2f),
                           ResourceManager::GetTexture("line"),
                           QVector3D(0.0f, 2.43f*0.5f+0.02f,  zField+0.5f));
    gameObjects.append(pBottomPole);
    pTopPole     = new Pole(QSizeF(2.43f, 0.2f),
                        ResourceManager::GetTexture("line"),
                        QVector3D(0.0f, 2.43f*0.5f+0.02f, -zField-0.5f));
    gameObjects.append(pTopPole);

    // Net White Bands
    QQuaternion q = QQuaternion::fromAxisAndAngle(QVector3D(-1.0f, 0.0f, 0.0f), 90.0f);
    pNetBandTop  = new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                           ResourceManager::GetTexture("line"),
                           QVector3D(0.0f, 2.43f+0.02f, 0.0f),
                           q,
                           QVector3D(0.2f, 1.0f, 1.0f));
    gameObjects.append(pNetBandTop);
    pNetBandBottom = new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                              ResourceManager::GetTexture("line"),
                              QVector3D(0.0f, 1.43f+0.02f, 0.0f),
                              q,
                              QVector3D(0.2f, 1.0f, 1.0f));
    gameObjects.append(pNetBandBottom);
    pNetBandLeft = new Pole(QSizeF(1.0f, 0.05f),
                            ResourceManager::GetTexture("line"),
                            QVector3D(0.0f, 1.93f+0.02f, -zField),
                            QQuaternion(),
                            QVector3D(0.2f, 1.0f, 1.0f));
    gameObjects.append(pNetBandLeft);
    pNetBandRight = new Pole(QSizeF(1.0f, 0.05f),
                             ResourceManager::GetTexture("line"),
                             QVector3D(0.0f, 1.93f+0.02f, zField),
                             QQuaternion(),
                             QVector3D(0.2f, 1.0f, 1.0f));
    gameObjects.append(pNetBandRight);
    // Horizontal wires
    for(int i=0; i<8; i++) {
        hRopes.append(new Pole(QSizeF(2.0f*zField, 0.01f),
                               ResourceManager::GetTexture("corda"),
                               QVector3D(0.0f, 2.43f-0.03f-((i+1)*0.1f), 0.0f),
                               q,
                               QVector3D(1.0f, 1.0f, 1.0f)));
        gameObjects.append(hRopes.last());
    }
    // Verticalal wires
    for(int i=0; i<int(zField*10.0)-1; i++) {
        vRopes.append(new Pole(QSizeF(0.9f, 0.01f),
                               ResourceManager::GetTexture("corda"),
                               QVector3D(0.0f, 1.93f+0.02f, (i+1)*0.1f),
                               QQuaternion(),
                               QVector3D(1.0f, 1.0f, 1.0f)));
        gameObjects.append(vRopes.last());
    }
    for(int i=0; i<int(zField*10.0)-1; i++) {
        vRopes.append(new Pole(QSizeF(0.9f, 0.01f),
                               ResourceManager::GetTexture("corda"),
                               QVector3D(0.0f, 1.93f+0.02f, -(i+1)*0.1f),
                               QQuaternion(),
                               QVector3D(1.0f, 1.0f, 1.0f)));
        gameObjects.append(vRopes.last());
    }
    vRopes.append(new Pole(QSizeF(0.9f, 0.01f),
                           ResourceManager::GetTexture("corda"),
                           QVector3D(0.0f, 1.93f+0.02f, 0.0f),
                           QQuaternion(),
                           QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(vRopes.last());

    // Now the Team Avatars
    pTeam0       = new Avatar(ballRadius,
                        ResourceManager::GetTexture("team0"),
                        QVector3D(-xField, ballRadius, z0Start));
    gameObjects.append(pTeam0);
    pTeam1       = new Avatar(ballRadius,
                        ResourceManager::GetTexture("team1"),
                        QVector3D(-xField, ballRadius, z1Start));
    gameObjects.append(pTeam1);

    pParticles = new ParticleGenerator(ResourceManager::GetTexture("particle"), 500);
}


void
RaceWidget::initShaders() {
    ResourceManager::LoadShader(":/Shaders/vRace.glsl",
                                ":/Shaders/fRace.glsl",
                                QString(),
                                "race");
    ResourceManager::LoadShader(":/Shaders/vDepth.glsl",
                                ":/Shaders/fDepth.glsl",
                                QString(),
                                "depth");
    ResourceManager::LoadShader(":/Shaders/vParticle.glsl",
                                ":/Shaders/fParticle.glsl",
                                QString(),
                                "particle");
#ifdef SHOW_DEPTH
    ResourceManager::LoadShader(":/Shaders/vDebug_quad.glsl",
                                ":/Shaders/vDebug_quad.glsl",
                                QString(),
                                "debug");
#endif
}


void
RaceWidget::initTextures() {
    ResourceManager::LoadTexture(":/VolleyBall_0.png", "team0");
    ResourceManager::LoadTexture(":/VolleyBall_1.png", "team1");
    ResourceManager::LoadTexture(":/blue-carpet.jpg",  "field");
    ResourceManager::LoadTexture(":/wood.png",         "floor");
    ResourceManager::LoadTexture(":/white-carpet.jpg", "line");
    ResourceManager::LoadTexture(":/corda_nera.jpg",   "corda");
    ResourceManager::LoadTexture(":/white-carpet.jpg", "particle");
}


void
RaceWidget::initShadowBuffer() {
    // Framebuffer with texture for shadows...
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
}


void
RaceWidget::initializeGL() {
    initializeOpenGLFunctions();

    initShaders();
    initTextures();
    initShadowBuffer();
    initGameObjects();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable(GL_MULTISAMPLE);

    refreshTime = 15; // in ms
    timer.start(refreshTime, this);
    t0 = QTime::currentTime().msecsSinceStartOfDay();
}


void
RaceWidget::startRace(int iSet) {
    if(maxScore[iSet] == 0) {
        return;
    }
    bRacing = true;
    bFireWorks = false;
    iCurrentSet = iSet;
    pTeam0->setPos(QVector3D(-xField, ballRadius, z0Start));
    pTeam1->setPos(QVector3D(-xField, ballRadius, z1Start));
    indexScore = 0;
    if(score[iCurrentSet].at(indexScore+1).x() > score[iCurrentSet].at(indexScore).x()) {
        pTeam0->setSpeed(QVector3D(speed, 0.0f, 0.0f));
        pTeam1->setSpeed(QVector3D(0.0f,  0.0f, 0.0f));
        teamMoving = 0;
    }
    else {
        pTeam0->setSpeed(QVector3D(0.0f,  0.0f, 0.0f));
        pTeam1->setSpeed(QVector3D(speed, 0.0f, 0.0f));
        teamMoving = 1;
    }
    xTarget = (2.0*xField)/float(maxScore[iCurrentSet]) - xField;
    emit newScore(score[iCurrentSet].at(indexScore+1).x(),
                  score[iCurrentSet].at(indexScore+1).y());
}


void
RaceWidget::paintGL() {
    pComputeDepthProgram = ResourceManager::GetShader("depth");
    pComputeDepthProgram->bind();
    pComputeDepthProgram->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glCullFace(GL_FRONT); // To fix peter panning
    glActiveTexture(GL_TEXTURE0);
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->draw(pComputeDepthProgram);
    }
    pComputeDepthProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    glViewport(0, 0, width(), height());
    glCullFace(GL_BACK); // Reset right culling face
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameProgram = ResourceManager::GetShader("race");
    pGameProgram->bind();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    pGameProgram->setUniformValue("view",             cameraProjectionMatrix);
    pGameProgram->setUniformValue("camera",           cameraViewMatrix);
    pGameProgram->setUniformValue("viewPos",          cameraPosition.toVector3D());
    pGameProgram->setUniformValue("lightPos",         lightPosition.toVector3D());
    pGameProgram->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
    pGameProgram->setUniformValue("diffuseTexture",   0);
    pGameProgram->setUniformValue("shadowMap",        1);

#ifndef SHOW_DEPTH
    renderScene(pGameProgram);
#endif

#ifdef SHOW_DEPTH
// render Depth map to quad for visual debugging
// ---------------------------------------------
    pDebugDepthQuad->bind();
    pDebugDepthQuad->setUniformValue("near_plane", near_plane);
    pDebugDepthQuad->setUniformValue("far_plane",  far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderQuad();
#endif
}


void
RaceWidget::renderScene(QOpenGLShaderProgram* pProgram) {
    glActiveTexture(GL_TEXTURE0);
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->draw(pProgram);
    }
    if(bFireWorks)
        pParticles->draw(pProgram);
}


void
RaceWidget::renderQuad() {
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
    if (quadVAO == 0)     {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    } // if (quadVAO == 0)

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void
RaceWidget::onStopFireworks() {
    fireworksTimer.stop();
    regenerateParticles = 0;
    closeTimer.start(5000);
}


void
RaceWidget::onTimeToClose() {
    closeTimer.stop();
    bRacing = false;
    bFireWorks = false;
    emit raceDone();
}


void
RaceWidget::timerEvent(QTimerEvent*) {
    if(bRacing) {
        float xCurrent = teamMoving ? pTeam1->getPos().x() : pTeam0->getPos().x();
        if(xCurrent >= xTarget) {
            indexScore++;
            if(indexScore > score[iCurrentSet].count()-2) {
                QVector3D origin = teamMoving ? pTeam1->getPos() : pTeam0->getPos();
                pTeam0->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
                pTeam1->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
                pParticles->init(origin);
                bRacing = false;
                bFireWorks = true;
                // Start Fireworks...
                regenerateParticles = 2;
                fireworksTimer.start(5000);
                update();
                return;
            }
            if(score[iCurrentSet].at(indexScore+1).x() >
               score[iCurrentSet].at(indexScore).x()) {
                teamMoving = 0;
                pTeam0->setSpeed(QVector3D(speed, 0.0f, 0.0f));
                pTeam1->setSpeed(QVector3D(0.0f,  0.0f, 0.0f));
                xTarget = score[iCurrentSet].at(indexScore+1).x()*(2.0*xField)/float(maxScore[iCurrentSet])-xField;
            }
            else {
                teamMoving = 1;
                pTeam0->setSpeed(QVector3D(0.0f,  0.0f, 0.0f));
                pTeam1->setSpeed(QVector3D(speed, 0.0f, 0.0f));
                xTarget = score[iCurrentSet].at(indexScore+1).y()*(2.0*xField)/float(maxScore[iCurrentSet])-xField;
            }
            emit newScore(score[iCurrentSet].at(indexScore+1).x(),
                          score[iCurrentSet].at(indexScore+1).y());
        }
    }
    t1 = QTime::currentTime().msecsSinceStartOfDay();
    float dt = (t1-t0)/1000.0;
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->updateStatus(dt);
    }
    if(bFireWorks)
        pParticles->Update(dt, regenerateParticles, QVector3D(0.0f, ballRadius, 0.0f));
    t0 = t1;
    update();
}

