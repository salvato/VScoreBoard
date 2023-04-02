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


#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QIcon>
#include <QTime>


//#define SHOW_DEPTH


RaceWidget::RaceWidget()
    : QOpenGLWidget()
{

// TODO: Remove
//    QList<QScreen*> screens = QApplication::screens();
//    QRect screenres = screens.at(0)->geometry();
//    if(screens.count() > 1) {
//        screenres = screens.at(1)->geometry();
//        QPoint point = QPoint(screenres.x(), screenres.y());
//        move(point);
//    }

    setWindowIcon(QIcon(":/buttonIcons/plot.png"));
    sTeamName[0] = "Locali";
    sTeamName[1] = "Ospiti";

    ballRadius = 0.1066f * 4.0f; // 4 times bigger than real

    diffuseColor  = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

//    cameraPosition = QVector4D(0.0f, 10.0f, 10.0f, 1.0f);
    cameraPosition = QVector4D(-1.5f*xField, 10.0f, 0.0f, 1.0f);
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
    connect(&closeTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToClose()));
}


RaceWidget::~RaceWidget() {
    makeCurrent();
    doneCurrent();
}


void
RaceWidget::closeEvent(QCloseEvent* event) {
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
RaceWidget::initializeGL() {
    initializeOpenGLFunctions();
    gameObjects.clear();
    pFloor       = new Floor(QSizeF(50.0f, 50.0f));
    gameObjects.append(pFloor);

    // Slightltly higer than Floor
    pPlayField   = new PlayField(QSizeF(xField, zField), QVector3D(0.0f, 0.01f, 0.0f));
    gameObjects.append(pPlayField);

    // Slightltly higer than PlayField
    pLeftLine    = new WhiteLine(QSizeF(0.05f, zField), QVector3D(-xField+0.05f, 0.02f, 0.0f));
    gameObjects.append(pLeftLine);
    pLeft3mLine  = new WhiteLine(QSizeF(0.05f, zField), QVector3D(-3.0f+0.05f, 0.02f, 0.0f));
    gameObjects.append(pLeft3mLine);
    pCentralLine = new WhiteLine(QSizeF(0.05f, zField), QVector3D(0.0f, 0.02f, 0.0f));
    gameObjects.append(pCentralLine);
    pRight3mLine = new WhiteLine(QSizeF(0.05f, zField), QVector3D(3.0f-0.05f, 0.02f, 0.0f));
    gameObjects.append(pRight3mLine);
    pRightLine   = new WhiteLine(QSizeF(0.05f, zField), QVector3D(xField-0.05f, 0.02f, 0.0f));
    gameObjects.append(pRightLine);
    pBottomLine  = new WhiteLine(QSizeF(xField, 0.05f), QVector3D(0.0f, 0.02f,  zField-0.05f));
    gameObjects.append(pBottomLine);
    pTopLine     = new WhiteLine(QSizeF(xField, 0.05f), QVector3D(0.0f, 0.02f, -zField+0.05f));
    gameObjects.append(pTopLine);
    pBottomPole  = new Pole(QSizeF(2.43f, 0.2f), QVector3D(0.0f, 2.43f*0.5f+0.02f,  zField+0.5f));
    gameObjects.append(pBottomPole);
    pTopPole     = new Pole(QSizeF(2.43f, 0.2f), QVector3D(0.0f, 2.43f*0.5f+0.02f, -zField-0.5f));
    gameObjects.append(pTopPole);

    QQuaternion q = QQuaternion::fromAxisAndAngle(QVector3D(-1.0f, 0.0f, 0.0f), 90.0f);
    pNetBand     = new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                            QVector3D(0.0f, 2.43f+0.02f, 0.0f),
                            q,
                            QVector3D(0.2f, 1.0f, 1.0f));
    gameObjects.append(pNetBand);

    pTeam0       = new Avatar(ballRadius, QVector3D(-xField, ballRadius, z0Start));
    gameObjects.append(pTeam0);
    pTeam1       = new Avatar(ballRadius, QVector3D(-xField, ballRadius, z1Start));
    gameObjects.append(pTeam1);

    for(int i=0; i<9; i++) {
        hRopes.append(new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                               QVector3D(0.0f, 2.43f-0.03f-((i+1)*0.1f), 0.0f),
                               q,
                               QVector3D(0.01f, 1.0f, 1.0f)));
        gameObjects.append(hRopes.at(i));
    }

    initShaders();
    initTextures();
    initShadowBuffer();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable(GL_MULTISAMPLE);
}


void
RaceWidget::initShaders() {
    pGameProgram = new QOpenGLShaderProgram();
    pGameProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/vRace.glsl");
    pGameProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fRace.glsl");
    pGameProgram->link();

    pComputeDepthProgram = new QOpenGLShaderProgram();
    pComputeDepthProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/vDepth.glsl");
    pComputeDepthProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fDepth.glsl");
    pComputeDepthProgram->link();

    pDebugDepthQuad = new QOpenGLShaderProgram();
    pDebugDepthQuad->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/vDebug_quad.glsl");
    pDebugDepthQuad->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fDebug_quad_depth.glsl");
    pDebugDepthQuad->link();
}


void
RaceWidget::initTextures() {
    pTeam0Texture = new QOpenGLTexture(QImage(":/VolleyBall_0.png").mirrored());
    pTeam0Texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pTeam0Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam0Texture->setWrapMode(QOpenGLTexture::Repeat);
    pTeam0->setTexture(pTeam0Texture);

    pTeam1Texture = new QOpenGLTexture(QImage(":/VolleyBall_1.png").mirrored());
    pTeam1Texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pTeam1Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam1Texture->setWrapMode(QOpenGLTexture::Repeat);
    pTeam1->setTexture(pTeam1Texture);

    pFieldTexture = new QOpenGLTexture(QImage(":/blue-carpet.jpg").mirrored());
    pFieldTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pFieldTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pFieldTexture->setWrapMode(QOpenGLTexture::Repeat);
    pPlayField->setTexture(pFieldTexture);

    pWoodTexture = new QOpenGLTexture(QImage(":/wood.png").mirrored());
    pWoodTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pWoodTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pWoodTexture->setWrapMode(QOpenGLTexture::Repeat);
    pFloor->setTexture(pWoodTexture);

    pLineTexture = new QOpenGLTexture(QImage(":/white-carpet.jpg").mirrored());
    pLineTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pLineTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pLineTexture->setWrapMode(QOpenGLTexture::Repeat);

    pLeftLine->setTexture(pLineTexture);
    pLeft3mLine->setTexture(pLineTexture);
    pCentralLine->setTexture(pLineTexture);
    pRight3mLine->setTexture(pLineTexture);
    pRightLine->setTexture(pLineTexture);
    pBottomLine->setTexture(pLineTexture);
    pTopLine->setTexture(pLineTexture);
    pBottomPole->setTexture(pLineTexture);
    pTopPole->setTexture(pLineTexture);
    pNetBand->setTexture(pLineTexture);

    pRopeTexture = new QOpenGLTexture(QImage(":/corda_nera.jpg").mirrored());
    for(int i=0; i<hRopes.count(); i++) {
        hRopes.at(i)->setTexture(pRopeTexture);
    }
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
RaceWidget::startRace(int iSet) {
    if(maxScore[iSet] == 0) {
        return;
    }
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
    refreshTime = 15; // in ms
    emit newScore(score[iCurrentSet].at(indexScore+1).x(),
                  score[iCurrentSet].at(indexScore+1).y());
    timer.start(refreshTime, this);
    t0 = QTime::currentTime().msecsSinceStartOfDay();
}


void
RaceWidget::paintGL() {
    pComputeDepthProgram->bind();
    pComputeDepthProgram->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glCullFace(GL_FRONT); // To fix peter panning
    renderScene(pComputeDepthProgram);
    pComputeDepthProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    glViewport(0, 0, width(), height());
    glCullFace(GL_BACK); // Reset right culling face
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
RaceWidget::renderPlayField(QOpenGLShaderProgram* pProgram) {
//    pProgram->setUniformValue("model", pPlayField->modelMatrix());
//    pFieldTexture->bind();
    pPlayField->draw(pProgram);

//    pLineTexture->bind();
//    pProgram->setUniformValue("model", pCentralLine->modelMatrix());
    pCentralLine->draw(pProgram);

//    pProgram->setUniformValue("model", pLeftLine->modelMatrix());
    pLeftLine->draw(pProgram);

//    pProgram->setUniformValue("model", pLeft3mLine->modelMatrix());
    pLeft3mLine->draw(pProgram);

//    pProgram->setUniformValue("model", pRightLine->modelMatrix());
    pRightLine->draw(pProgram);

//    pProgram->setUniformValue("model", pRight3mLine->modelMatrix());
    pRight3mLine->draw(pProgram);

//    pProgram->setUniformValue("model", pBottomLine->modelMatrix());
    pBottomLine->draw(pProgram);

//    pProgram->setUniformValue("model", pTopLine->modelMatrix());
    pTopLine->draw(pProgram);

//    pProgram->setUniformValue("model", pTopPole->modelMatrix());
    pTopPole->draw(pProgram);

//    pProgram->setUniformValue("model", pBottomPole->modelMatrix());
    pBottomPole->draw(pProgram);

//    pProgram->setUniformValue("model", pNetBand->modelMatrix());
    pNetBand->draw(pProgram);
}


void
RaceWidget::renderScene(QOpenGLShaderProgram* pProgram) {
    glActiveTexture(GL_TEXTURE0);

    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->draw(pProgram);
    }
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
RaceWidget::onTimeToClose() {
    closeTimer.stop();
    emit raceDone();
}


void
RaceWidget::timerEvent(QTimerEvent*) {
    float xCurrent = teamMoving ? pTeam1->getPos().x() : pTeam0->getPos().x();
    if(xCurrent >= xTarget) {
        indexScore++;
        if(indexScore > score[iCurrentSet].count()-2) {
            timer.stop();
            pTeam0->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
            pTeam1->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
            closeTimer.start(3000);
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
    t1 = QTime::currentTime().msecsSinceStartOfDay();
    float t = (t1-t0)/1000.0;
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->updateStatus(t);
    }
    t0 = t1;
    update();
}

