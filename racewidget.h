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


QT_FORWARD_DECLARE_CLASS(PlayField)
QT_FORWARD_DECLARE_CLASS(WhiteLine)
QT_FORWARD_DECLARE_CLASS(Avatar)
QT_FORWARD_DECLARE_CLASS(Pole)
QT_FORWARD_DECLARE_CLASS(Floor)


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
    void startRace(int iSet);

public slots:
    void closeEvent(QCloseEvent*) override;
    void onTimeToClose();

signals:
    void raceDone();
    void newScore(int score0, int score1);

protected:
    void timerEvent(QTimerEvent *e) override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void renderScene(QOpenGLShaderProgram *pProgram);
    void renderPlayField(QOpenGLShaderProgram* pProgram);
    void renderQuad();
    void ConfigureModelMatrices();

    void initShaders();
    void initTextures();

private:
//    struct VertexData {
//        QVector3D position;
//        QVector2D texCoord;
//    };
//    QOpenGLBuffer fieldBuf;

    QBasicTimer timer;
    QTimer closeTimer;

    QOpenGLTexture* pEnvironment   = nullptr;
    QOpenGLTexture* pTeam0Texture  = nullptr;
    QOpenGLTexture* pTeam1Texture  = nullptr;
    QOpenGLTexture* pFieldTexture  = nullptr;
    QOpenGLTexture* pLineTexture   = nullptr;
    QOpenGLTexture* pWoodTexture   = nullptr;

    QOpenGLShaderProgram* pEnvironmentProgram = nullptr;
    QOpenGLShaderProgram* pGameProgram = nullptr;
    QOpenGLShaderProgram* pComputeDepthProgram = nullptr;
    QOpenGLShaderProgram* pDebugDepthQuad;

    QMatrix4x4 floorModelMatrix;

    QMatrix4x4 fieldModelMatrix;
    QMatrix4x4 centralLineModelMatrix;
    QMatrix4x4 leftLineModelMatrix;
    QMatrix4x4 left3mLineModelMatrix;
    QMatrix4x4 rightLineModelMatrix;
    QMatrix4x4 right3mLineModelMatrix;
    QMatrix4x4 bottomLineModelMatrix;
    QMatrix4x4 topLineModelMatrix;
    QMatrix4x4 bottomPoleModelMatrix;
    QMatrix4x4 topPoleModelMatrix;

    QMatrix4x4 team0ModelMatrix;
    QMatrix4x4 team1ModelMatrix;

    QMatrix4x4 cameraViewMatrix;
    QMatrix4x4 cameraProjectionMatrix;
    QMatrix4x4 translateMatrix;
    QMatrix4x4 lightProjectionMatrix;
    QMatrix4x4 lightViewMatrix;
    QMatrix4x4 lightSpaceMatrix;

    QVector4D lightPosition;
    QVector4D diffuseColor;
    QVector4D specularColor;
    QVector4D cameraPosition;

    Floor*     pFloor;
    PlayField* pPlayField;
    WhiteLine* pCentralLine;
    WhiteLine* pXLine;
    WhiteLine* pZLine;
    Pole*      pPole;
    Avatar*    pTeam0;
    Avatar*    pTeam1;

    QVector3D rotationAxis;
    QQuaternion rotation1;
    QQuaternion rotation0;
    float xCamera =  0.0;
    float yCamera = 15.0;
    float zCamera = 20.0;
    float z0Start =  2.0;
    float z1Start = -2.0;
    float xField  =  9.0;
    float zField  =  4.5;
    int   nVertices;
    float ballRadius;
    double dx;
    double dx0;
    double dx1;
    double x0;
    double x1;
    QString sTeamName[2];
    QVector<QVector2D> score[5];
    int maxScore[5];
    int indexScore;
    int iCurrentSet;
    float scanTime;
    float pointTime;
    float pointSpace;
    double t0;
    int refreshTime;
    const unsigned int SHADOW_WIDTH  = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    unsigned int depthMap;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    float near_plane = 0.0f;
    float far_plane  = 7.5f;
    QVector<float> floorVertices;
    unsigned int floorVAO;
};

