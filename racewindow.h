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
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QCloseEvent>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QBasicTimer>


QT_FORWARD_DECLARE_CLASS(TeamAvatar)
QT_FORWARD_DECLARE_CLASS(Sphere)


class RaceWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    RaceWindow();
    ~RaceWindow();

public slots:
    void closeEvent(QCloseEvent*) override;

protected:
    void mousePressEvent(QMouseEvent* pEvent) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;
    void wheelEvent(QWheelEvent* pEvent) override;

    void timerEvent(QTimerEvent *e) override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initEnvironment();
    void initPlayField();
    void drawField(QOpenGLShaderProgram* pProgram);

    void initShaders();
    void initTextures();

private:
    struct VertexData {
        QVector3D position;
        QVector2D texCoord;
    };
    QOpenGLBuffer fieldBuf;

    QBasicTimer timer;

    QOpenGLTexture* pEnvironment = nullptr;
    QOpenGLTexture* pAvatar0Texture = nullptr;
    QOpenGLTexture* pAvatar1Texture = nullptr;
    QOpenGLTexture* pFieldTexture   = nullptr;
    QOpenGLTexture* pSphereTexture  = nullptr;

    QOpenGLShaderProgram* pEnvironmentProgram = nullptr;
    QOpenGLShaderProgram* pAvatarProgram = nullptr;
    QOpenGLShaderProgram* pFieldProgram = nullptr;
    QOpenGLShaderProgram* pSphereProgram = nullptr;

    QMatrix4x4 modelMatrix;
    QMatrix4x4 cameraMatrix;
    QMatrix4x4 viewMatrix;
    QMatrix4x4 translateMatrix;

    QVector4D lightPosition;

    TeamAvatar* pTeam0;
    TeamAvatar* pTeam1;
    TeamAvatar* pPlayField;
    Sphere*     pSphere;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed = 0;
    QQuaternion rotation;
    float xCamera =  0.0;
    float yCamera = 15.0;
    float zCamera =-20.0;
    float z0Start =  1.0;
    float z1Start = -1.0;
    float xField  =  9.0;
    float zField  =  4.5;
    int nVertices;
};

