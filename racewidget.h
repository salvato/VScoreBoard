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
#pragma once

#include "model3d.h"
#include "model3dex.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QCloseEvent>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QBasicTimer>
#include <QTimer>
#include <QTime>


QT_FORWARD_DECLARE_CLASS(Object)
QT_FORWARD_DECLARE_CLASS(PlayField)
QT_FORWARD_DECLARE_CLASS(WhiteLine)
QT_FORWARD_DECLARE_CLASS(Avatar)
QT_FORWARD_DECLARE_CLASS(Pole)
QT_FORWARD_DECLARE_CLASS(Floor)
QT_FORWARD_DECLARE_CLASS(ParticleGenerator)


class RaceWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    RaceWidget();
    ~RaceWidget();

public:
    void updateScore(int team0Score, int team1Score, int iSet);
    void updateLabel(int iTeam, QString sLabel);
    void resetScore(int iSet);
    void resetAll();
    void resetInitialStatus();
    void startRace(int iSet);

public slots:
    void closeEvent(QCloseEvent*) override;
    void hideEvent(QHideEvent *event) override;
    void onStopFireworks();
    void onTimeToClose();

signals:
    void raceDone();
    void newScore(int score0, int score1);

protected:
    void timerEvent(QTimerEvent *e) override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void renderScene();
    void renderScene(QOpenGLShaderProgram *pProgram);
    void renderQuad(QOpenGLShaderProgram *pProgram);

    void initShaders();
    void initShadowBuffer();
    void initTextures();
    void initGameObjects();
    void restoreStatus();
    void createWall();
    void createFloor();
    void createNet();
    void createTextModel();
    float* generateVertexBuffer(const Model3D& model);
    void generateNormalsBuffer(const Model3D& model, float* coords);
    void create2DImage(const QString& sText, const QFont& font, ushort height);

private:
    QBasicTimer timerUpdate;
    QTimer closeTimer;
    QTimer fireworksTimer;
    int scanTime;
    int closeTime;
    int refreshTime;

    QOpenGLShaderProgram* pGameProgram = nullptr;
    QOpenGLShaderProgram* pComputeDepthProgram = nullptr;
    QOpenGLShaderProgram* pTextProgram = nullptr;
    QOpenGLShaderProgram* pDebugDepthQuad = nullptr;

    QMatrix4x4 cameraViewMatrix;
    QMatrix4x4 cameraProjectionMatrix;

    QMatrix4x4 textProjectionMatrix;

    QMatrix4x4 lightProjectionMatrix;
    QMatrix4x4 lightViewMatrix;
    QMatrix4x4 lightSpaceMatrix;

    QVector3D light;
    QVector3D lightColor;
    QVector3D lightPosition;

    QVector3D cameraPosition0;
    QVector3D cameraCenter0;
    QVector3D cameraUp0;
    QVector3D cameraSpeed0;
    QVector3D cameraPosition;
    QVector3D cameraCenter;
    QVector3D cameraUp;
    QVector3D cameraSpeed;

    QVector<Object*> gameObjects;
    Avatar* pTeam0;
    Avatar* pTeam1;

    const float z0Start = -2.0;
    const float z1Start =  2.0;
    const float xField  =  9.0; // Half Width of the Play Field
    const float zField  =  4.5; // Half Height of the Play Field

    const uint SHADOW_WIDTH  = 1024;
    const uint SHADOW_HEIGHT = 1024;

    float ballRadius;

    float xCamera =  0.0;
    float yCamera = 15.0;
    float zCamera = 20.0;
    float near_plane = 0.0f;
    float far_plane  = 7.5f;

    QString sTeamName[2];
    QVector<QVector2D> score[5];
    int maxScore[5];
    int indexScore;
    int iCurrentSet;
    unsigned int depthMapFBO;
    unsigned int depthMap;
    QOpenGLBuffer* pQuadBuf = nullptr;

    float speed;
    QVector3D speed0;
    QVector3D speed1;
    float xTarget;
    int t0, t1;
    int teamMoving;

    ParticleGenerator* pParticles;
    int regenerateParticles;

    bool bRacing;
    bool bFiring;
    bool bClosing;
    bool bFadeIn;

    int fireworkTime;
    QVector3D origin;
    qreal zNear;
    qreal zFar;
    qreal fov;
    qreal aspect;

    QOpenGLBuffer vertexTextBuf;
    QOpenGLBuffer normalTextBuf;
    Model3DEx     textModel;
    QImage*       pTextImage = nullptr;
};

