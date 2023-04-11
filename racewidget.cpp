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
#include "whiteline.h"
#include "pole.h"
#include "avatar.h"
#include "particlegenerator.h"
#include "resourcemanager.h"


#include <QSurfaceFormat>
#include <QScreen>
#include <QIcon>
#include <QTime>


//#define SHOW_DEPTH


RaceWidget::RaceWidget()
    : QOpenGLWidget()
    , scanTime(10)
    , closeTime(3000)
    , lightColor(QVector3D(1.0f, 1.0f, 1.0f))
    , lightPosition(QVector3D(0.5f, 4.0f, -3.0f))
    , pTeam0(nullptr)
    , pTeam1(nullptr) // Tempo in secondi per l'intera "Corsa"
    , ballRadius(0.1066f * 4.0f) // 4 times bigger than real
    , speed(2.0f*xField/float(scanTime))
    , bRacing(false)
    , bFiring(false)
    , bClosing(false)
    , fireworkTime(5000)
    , zNear(0.01f)
    , zFar(30.0f)
    , fov(50.0f)
{
    setWindowIcon(QIcon(":/buttonIcons/plot.png"));

    if (FT_Init_FreeType(&ft))
    {
        qCritical() << "ERROR::FREETYPE: Could not init FreeType Library";
        exit(EXIT_FAILURE);
    }
    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
    {
        qCritical() << "ERROR::FREETYPE: Failed to load font";
        exit(EXIT_FAILURE);
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    sTeamName[0] = "Locali";
    sTeamName[1] = "Ospiti";
/*
    cameraPosition = QVector4D(0.0f, 5.0f, 0.0f, 1.0f);
    cameraViewMatrix.lookAt(cameraPosition.toVector3D(),   // Eye
                            QVector3D(xField, 0.0f, 0.0f), // Center
                            QVector3D(0.0f, 1.0f, 0.0f));  // Up
*/
    cameraPosition0 = QVector3D(-3.0, 10.0f, 10.0f);
    cameraCenter0   = QVector3D(cameraPosition0.x(), 0.0f, 0.25*zField);
    cameraUp0       = QVector3D( 0.0f,  1.0f,  0.0f);
    cameraSpeed0    = QVector3D(0.3f*speed,  0.0f,  0.0f);

    cameraPosition  = cameraPosition0;
    cameraCenter    = cameraCenter0;
    cameraUp        = cameraUp0;
    cameraSpeed     = cameraSpeed0;

    cameraViewMatrix.setToIdentity();
    cameraViewMatrix.lookAt(cameraPosition, // Eye
                            cameraCenter,   // Center
                            cameraUp);      // Up

    float extension = std::max(xField, zField)*1.5;
    near_plane = -1.0f;
    far_plane  = extension;
    lightProjectionMatrix.setToIdentity();
    lightProjectionMatrix.ortho(-extension, extension,
                                -extension, extension,
                                 near_plane, far_plane);
    lightViewMatrix.lookAt(lightPosition,                       // Eye
                           QVector3D(0.0f, 0.0f, 0.25f*zField), // Center
                           QVector3D(0.0f, 1.0f, 0.0f));        // Up
    lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

    resetAll();
    connect(&fireworksTimer, SIGNAL(timeout()),
             this, SLOT(onStopFireworks()));
    connect(&closeTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToClose()));
}


RaceWidget::~RaceWidget() {
    timerUpdate.stop();
    fireworksTimer.stop();
    closeTimer.stop();
    emit raceDone();
    makeCurrent();
    doneCurrent();
}


void
RaceWidget::closeEvent(QCloseEvent* event) {
    timerUpdate.stop();
    fireworksTimer.stop();
    closeTimer.stop();
    emit raceDone();
    makeCurrent();
    doneCurrent();
    event->accept();
}


void
RaceWidget::hideEvent(QHideEvent *event) {
    timerUpdate.stop();
    fireworksTimer.stop();
    closeTimer.stop();
    emit raceDone();
    event->accept();
}


void
RaceWidget::resetInitialStatus() {
    restoreStatus();
    if(pTeam0) {
        bFadeIn = true;
        pTeam0->setPos(QVector3D(-xField, ballRadius, z0Start));
        pTeam1->setPos(QVector3D(-xField, ballRadius, z1Start));
        pTeam0->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
        pTeam1->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
        if(!timerUpdate.isActive()) {
            timerUpdate.start(refreshTime, this);
            t0 = QTime::currentTime().msecsSinceStartOfDay();
        }
    }
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
     // Rimuoviamo eventuali punteggi sbagliati...
    int tScore = team0Score+team1Score;
    while((tScore <= score[iSet].last().x()+score[iSet].last().y()) &&
           !score[iSet].isEmpty())
    {
        score[iSet].removeLast();
    }
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
    aspect = qreal(w) / qreal(h ? h : 1);
    cameraProjectionMatrix.perspective(fov, aspect, zNear, zFar);
}

void
RaceWidget::createWall() {
    QQuaternion q = QQuaternion();
    q = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f);
    gameObjects.append(new Floor(QSizeF(25.0f, 2.5f),
                                 ResourceManager::GetTexture("brickwall"),
                                 QVector3D(0.0f, 2.5f, -10.0f),
                                 q));
    q  = QQuaternion::fromAxisAndAngle(QVector3D( 0.0f,-1.0f, 0.0f), 90.0f);
    q *= QQuaternion::fromAxisAndAngle(QVector3D( 1.0f, 0.0f, 0.0f), 90.0f);
    gameObjects.append(new Floor(QSizeF(25.0f, 2.5f),
                                 ResourceManager::GetTexture("brickwall"),
                                 QVector3D(25.0f, 2.5f, 0.0f),
                                 q));

    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D(24.9f, 1.5f,-4.0f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D(24.9f, 1.5f, 0.0f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D(24.9f, 1.5f, 4.0f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
}


void
RaceWidget::createFloor(){
    QQuaternion q = QQuaternion();
    // Wooden Floor
    gameObjects.append(new Floor(QSizeF(25.0f, 10.0f),
                                 ResourceManager::GetTexture("floor")));
    // Play Field (Slightltly higer than Floor)
    gameObjects.append(new Floor(QSizeF(xField, zField),
                                 ResourceManager::GetTexture("field"),
                                 QVector3D(0.0f, 0.01f, 0.0f)));
    gameObjects.append(new Floor(QSizeF(xField+5.0, zField+3.0),
                                 ResourceManager::GetTexture("field-ex"),
                                 QVector3D(0.0f, 0.005f, 0.0f)));

    // Loghi SSD
    q = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f);
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D(-4.0f, 1.5f, -9.9f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D( 4.0f, 1.5f, -9.9f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));

    // Loghi nel campo
    q = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, -1.0f, 0.0f), 90.0f);
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoUnime"),
                                 QVector3D(-xField+3.0f, 0.02f, 0.0f),
                                 q,
                                 QVector3D(1.5f, 1.0f, 1.5f)));
    q = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 90.0f);
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoUnime"),
                                 QVector3D(xField-3.0f, 0.02f, 0.0f),
                                 q,
                                 QVector3D(1.5f, 1.0f, 1.5f)));

    // Loghi esterni al campo
    q = QQuaternion();
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D(-5.0f, 0.02f, zField+1.5f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoUnime2"),
                                 QVector3D( 5.0f, 0.02f, zField+1.5f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));

    q = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 180.0f);
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoUnime2"),
                                 QVector3D(-5.0f, 0.02f,-zField-1.5f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));
    gameObjects.append(new Floor(QSizeF(1.0f, 1.0f),
                                 ResourceManager::GetTexture("logoSSD"),
                                 QVector3D( 5.0f, 0.02f,-zField-1.5f),
                                 q,
                                 QVector3D(1.0f, 1.0f, 1.0f)));

    // Lines (Slightly higer than PlayField)
    gameObjects.append(new WhiteLine(QSizeF(0.05f, zField),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(-xField+0.05f, 0.02f, 0.0f)));
    gameObjects.append(new WhiteLine(QSizeF(0.05f, zField),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(-3.0f+0.05f, 0.02f, 0.0f)));
    gameObjects.append(new WhiteLine(QSizeF(0.05f, zField),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(0.0f, 0.02f, 0.0f)));
    gameObjects.append(new WhiteLine(QSizeF(0.05f, zField),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(3.0f-0.05f, 0.02f, 0.0f)));
    gameObjects.append(new WhiteLine(QSizeF(0.05f, zField),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(xField-0.05f, 0.02f, 0.0f)));
    gameObjects.append(new WhiteLine(QSizeF(xField, 0.05f),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(0.0f, 0.02f,  zField-0.05f)));
    gameObjects.append(new WhiteLine(QSizeF(xField, 0.05f),
                                     ResourceManager::GetTexture("line"),
                                     QVector3D(0.0f, 0.02f, -zField+0.05f)));
}


void
RaceWidget::createNet() {
    QQuaternion q = QQuaternion();
    // Net White Bands
    q = QQuaternion::fromAxisAndAngle(QVector3D(-1.0f, 0.0f, 0.0f), 90.0f);
    gameObjects.append(new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 2.43f+0.02f, 0.0f),
                                q,
                                QVector3D(0.2f, 1.0f, 1.0f)));
    gameObjects.append(new Pole(QSizeF(2.0f*zField+1.0f, 0.05f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 1.43f+0.02f, 0.0f),
                                q,
                                QVector3D(0.2f, 1.0f, 1.0f)));
    gameObjects.append(new Pole(QSizeF(1.0f, 0.05f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 1.93f+0.02f, -zField),
                                QQuaternion(),
                                QVector3D(0.2f, 1.0f, 1.0f)));
    gameObjects.append(new Pole(QSizeF(1.0f, 0.05f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 1.93f+0.02f, zField),
                                QQuaternion(),
                                QVector3D(0.2f, 1.0f, 1.0f)));
    // Horizontal wires
    for(int i=0; i<8; i++) {
        gameObjects.append(new Pole(QSizeF(2.0f*zField, 0.01f),
                                    ResourceManager::GetTexture("corda"),
                                    QVector3D(0.0f, 2.43f-0.03f-((i+1)*0.1f), 0.0f),
                                    q,
                                    QVector3D(1.0f, 1.0f, 1.0f)));
    }
    // Vertical wires
    for(int i=0; i<int(zField*10.0)-1; i++) {
        gameObjects.append(new Pole(QSizeF(0.9f, 0.01f),
                                    ResourceManager::GetTexture("corda"),
                                    QVector3D(0.0f, 1.93f+0.02f, (i+1)*0.1f),
                                    QQuaternion(),
                                    QVector3D(1.0f, 1.0f, 1.0f)));
    }
    for(int i=0; i<int(zField*10.0)-1; i++) {
        gameObjects.append(new Pole(QSizeF(0.9f, 0.01f),
                                    ResourceManager::GetTexture("corda"),
                                    QVector3D(0.0f, 1.93f+0.02f, -(i+1)*0.1f),
                                    QQuaternion(),
                                    QVector3D(1.0f, 1.0f, 1.0f)));
    }
    gameObjects.append(new Pole(QSizeF(0.9f, 0.01f),
                                ResourceManager::GetTexture("corda"),
                                QVector3D(0.0f, 1.93f+0.02f, 0.0f),
                                QQuaternion(),
                                QVector3D(1.0f, 1.0f, 1.0f)));
}


void
RaceWidget::initChars() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for(unsigned char c=0; c<128; c++) {
        // load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            qCritical() << "ERROR::FREETYTPE: Failed to load Glyph";
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            QVector2D(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            QVector2D(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<uint>(face->glyph->advance.x)
        };
        Characters.insert(c, character);
    }
}


void
RaceWidget::initGameObjects() {
    QQuaternion q = QQuaternion();
    gameObjects.clear();
    createWall();
    createFloor();
    // Pali
    gameObjects.append(new Pole(QSizeF(2.43f, 0.2f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 2.43f*0.5f+0.02f,  zField+0.5f)));
    gameObjects.append(new Pole(QSizeF(2.43f, 0.2f),
                                ResourceManager::GetTexture("line"),
                                QVector3D(0.0f, 2.43f*0.5f+0.02f, -zField-0.5f)));
    createNet();

    // Now the Team Avatars...
    pTeam0       = new Avatar(ballRadius,
                        ResourceManager::GetTexture("team0"),
                        QVector3D(-xField, ballRadius, z0Start));
    gameObjects.append(pTeam0);
    pTeam1       = new Avatar(ballRadius,
                        ResourceManager::GetTexture("team1"),
                        QVector3D(-xField, ballRadius, z1Start));
    gameObjects.append(pTeam1);

    // ...and then the Particles
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
                                ":/Shaders/fDebug_quad.glsl",
                                QString(),
                                "debug");
#endif
}


void
RaceWidget::initTextures() {
    ResourceManager::LoadTexture(":/VolleyBall_0.png",   "team0");
    ResourceManager::LoadTexture(":/VolleyBall_1.png",   "team1");
    ResourceManager::LoadTexture(":/blue-carpet.jpg",    "field");
    ResourceManager::LoadTexture(":/dark-blue-carpet.jpg", "field-ex");
    ResourceManager::LoadTexture(":/wood.png",           "floor");
    ResourceManager::LoadTexture(":/white-carpet.jpg",   "line");
    ResourceManager::LoadTexture(":/corda_nera.jpg",     "corda");
    ResourceManager::LoadTexture(":/white-carpet.jpg",   "particle");
    ResourceManager::LoadTexture(":/Logo_UniMe.png",     "logoUnime");
    ResourceManager::LoadTexture(":/Logo_UniMe2.png",    "logoUnime2");
    ResourceManager::LoadTexture(":/Logo_SSD_UniMe.png", "logoSSD");
    ResourceManager::LoadTexture(":/brick-wall.jpg",     "brickwall");
    ResourceManager::LoadTexture(":/Logo_SSD.png",       "ssd");
    ResourceManager::LoadTexture(":/gate0.png",          "gate");
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    refreshTime = 15; // in ms
    timerUpdate.start(refreshTime, this);
    t0 = QTime::currentTime().msecsSinceStartOfDay();
}


void
RaceWidget::startRace(int iSet) {
    if(maxScore[iSet] == 0) {
        emit raceDone();
        return;
    }
    bFadeIn     = false;
    bRacing     = true;
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
    if(!timerUpdate.isActive()) {
        timerUpdate.start(refreshTime, this);
        t0 = QTime::currentTime().msecsSinceStartOfDay();
    }
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
    renderScene(pComputeDepthProgram);
    pComputeDepthProgram->release();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    glViewport(0, 0, width(), height());
    glCullFace(GL_BACK); // Reset right culling face
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameProgram = ResourceManager::GetShader("race");
    pGameProgram->bind();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    pGameProgram->setUniformValue("view",              cameraProjectionMatrix);
    pGameProgram->setUniformValue("camera",            cameraViewMatrix);
    pGameProgram->setUniformValue("viewPos",           cameraPosition);
    pGameProgram->setUniformValue("lightPos",          lightPosition);
    pGameProgram->setUniformValue("lightColor",        light);
    pGameProgram->setUniformValue("lightSpaceMatrix",  lightSpaceMatrix);
    pGameProgram->setUniformValue("diffuseTexture",    0);
    pGameProgram->setUniformValue("shadowMap",         1);

#ifndef SHOW_DEPTH
    pTextProgram = ResourceManager::GetShader("text");
    renderText(pTextProgram, "This is sample text", 25.0f, 25.0f, 1.0f, QVector3D(0.5, 0.8f, 0.2f));
    renderText(pTextProgram, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, QVector3D(0.3, 0.7f, 0.9f));
    renderScene(pGameProgram);
#endif

#ifdef SHOW_DEPTH
// render Depth map to quad for visual debugging
// ---------------------------------------------
    pDebugDepthQuad = ResourceManager::GetShader("debug");
    pDebugDepthQuad->bind();
    pDebugDepthQuad->setUniformValue("near_plane", near_plane);
    pDebugDepthQuad->setUniformValue("far_plane",  far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderQuad(pDebugDepthQuad);
#endif
}


void
RaceWidget::renderScene(QOpenGLShaderProgram* pProgram) {
    glActiveTexture(GL_TEXTURE0);
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->draw(pProgram);
    }
    if(bFiring)
        pParticles->draw(pProgram);
}


void
RaceWidget::renderQuad(QOpenGLShaderProgram* pProgram) {
    (void)pProgram;
#ifdef SHOW_DEPTH
    if(pQuadBuf == nullptr) {
        pQuadBuf = new QOpenGLBuffer();
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        pQuadBuf->create();
        pQuadBuf->bind();
        pQuadBuf->allocate(quadVertices, sizeof(quadVertices));
        pQuadBuf->release();
    } // if(pQuadBuf == nullptr)

    pQuadBuf->bind();
    quintptr offset = 0;
    pProgram->enableAttributeArray("vPosition");
    pProgram->setAttributeBuffer("vPosition", GL_FLOAT, offset, 3, 5*sizeof(float));
    offset += 3*sizeof(float);
    pProgram->enableAttributeArray("vTexture");
    pProgram->setAttributeBuffer("vTexture", GL_FLOAT, offset, 2, 5*sizeof(float));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    pQuadBuf->release();
#endif
}


void
RaceWidget::onStopFireworks() {
    fireworksTimer.stop();
    regenerateParticles = 0;
    bClosing = true;
    closeTimer.start(closeTime);
}


void
RaceWidget::onTimeToClose() {
    closeTimer.stop();
    emit raceDone();
    restoreStatus();
}


void
RaceWidget::restoreStatus() {
    emit newScore(0, 0);
    bFadeIn  = true;
    bRacing  = false;
    bFiring  = false;
    bClosing = false;
    light    = QVector3D(0.0f, 0.0f, 0.0f);
    fov      = 50;
    cameraProjectionMatrix.setToIdentity();
    cameraProjectionMatrix.perspective(fov, aspect, zNear, zFar);
    cameraPosition = cameraPosition0;
    cameraCenter   = cameraCenter0;
    cameraUp       = cameraUp0;
    cameraSpeed    = cameraSpeed0;
    cameraViewMatrix.setToIdentity();
    cameraViewMatrix.lookAt(cameraPosition, // Eye
                            cameraCenter,   // Center
                            cameraUp);      // Up
}


void
RaceWidget::timerEvent(QTimerEvent*) {
    t1 = QTime::currentTime().msecsSinceStartOfDay();
    float dt = (t1-t0)/1000.0;
    if(bRacing) {
        float xCurrent = teamMoving ? pTeam1->getPos().x() : pTeam0->getPos().x();
        if(xCurrent >= xTarget) {
            indexScore++;
            if(indexScore > score[iCurrentSet].count()-2) {
                origin = teamMoving ? pTeam1->getPos() : pTeam0->getPos();
                pTeam0->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
                pTeam1->setSpeed(QVector3D(0.0f, 0.0f, 0.0f));
                pParticles->init(origin);
                bRacing = false;
                bFiring = true;
                // Start Fireworks...
                regenerateParticles = 2;
                fireworksTimer.start(fireworkTime);
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
        cameraPosition += cameraSpeed*dt;
        cameraCenter.setX(cameraPosition.x());
        cameraViewMatrix.setToIdentity();
        cameraViewMatrix.lookAt(cameraPosition, // Eye
                                cameraCenter,   // Center
                                cameraUp);      // Up
    }
    for(int i=0; i<gameObjects.count(); i++) {
        gameObjects.at(i)->updateStatus(dt);
    }
    if(bFiring) {
        pParticles->Update(dt, regenerateParticles, QVector3D(0.0f, ballRadius, 0.0f));
        if(cameraCenter.x() < origin.x()) {
            cameraCenter.setX(cameraCenter.x()+0.25*speed*dt);
        }
        if(cameraCenter.z() > origin.z()) {
            cameraCenter.setZ(cameraCenter.z()-0.25*speed*dt);
        }
        if(cameraCenter.z() < origin.z()) {
            cameraCenter.setZ(cameraCenter.z()+0.25*speed*dt);
        }
        cameraViewMatrix.setToIdentity();
        cameraViewMatrix.lookAt(cameraPosition, // Eye
                                cameraCenter,   // Center
                                cameraUp);      // Up
    }
    if(bClosing) {
        fov   *= 0.999;
        light *= 0.97f;
        cameraProjectionMatrix.setToIdentity();
        cameraProjectionMatrix.perspective(fov, aspect, zNear, zFar);
    }
    if(bFadeIn) {
        light += lightColor*dt/3.3f;
    }
    t0 = t1;
    update();
}

