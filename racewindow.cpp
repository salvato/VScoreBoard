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

#include "racewindow.h"
#include "floor.h"
#include "playfield.h"
#include "sphere.h"


#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QIcon>
#include <QTime>

/*
const static char
environmentVShaderText[] =
"varying vec3 position, normal;"
"varying vec4 specular, ambient, diffuse, lightDirection;"

"uniform mat4 view;"

"void main()"
"{"
    "gl_TexCoord[0] = gl_MultiTexCoord0;"
    "gl_TexCoord[1] = gl_Vertex;"
    "specular = gl_LightSource[0].specular;"
    "ambient = gl_LightSource[0].ambient;"
    "diffuse = gl_LightSource[0].diffuse;"
    "lightDirection = view * gl_LightSource[0].position;"

    "normal = gl_NormalMatrix * gl_Normal;"
    "position = (gl_ModelViewMatrix * gl_Vertex).xyz;"

    "gl_FrontColor = gl_Color;"
    "gl_Position = ftransform();"
"}";



const static char
environmentFShaderText[] =
    "uniform samplerCube env;"
    "void main() {"
        "gl_FragColor = textureCube(env, gl_TexCoord[1].xyz);"
    "}";

*/

//#define SHOW_DEPTH


RaceWindow::RaceWindow()
    : QOpenGLWidget()
    , fieldBuf(QOpenGLBuffer::VertexBuffer)
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

    ballRadius = 0.1066f*4.0f; // 4 times bigger than real

    diffuseColor  = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

    cameraPosition = QVector4D(0.0f, 10.0f, 10.0f, 1.0f);
    cameraViewMatrix.lookAt(cameraPosition.toVector3D(),        // Eye
                            QVector3D(0.0f,    0.0f,    0.0f),  // Center
                            QVector3D(0.0f,    1.0f,    0.0f)); // Up

    lightPosition = QVector4D(-2.0f, 4.0f, -1.0f, 1.0f);

    float extension = std::max(xField, zField)*1.5;
    near_plane = -1.0f;
    far_plane  = extension;
    lightProjectionMatrix.setToIdentity();
    lightProjectionMatrix.ortho(-extension, extension, -extension, extension, near_plane, far_plane);
    lightViewMatrix.lookAt(lightPosition.toVector3D(),     // Eye
                           QVector3D( 0.0f, 0.0f,  0.0f),  // Center
                           QVector3D( 0.0f, 1.0f,  0.0f)); // Up
    lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

    resetAll();
    scanTime = 15.0; // Tempo in secondi per l'intera "Corsa"
    x0  = x1  =-xField;
    dx0 = dx1 = 0;
    connect(&closeTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToClose()));
}


RaceWindow::~RaceWindow() {
    makeCurrent();
    doneCurrent();
}


void
RaceWindow::closeEvent(QCloseEvent* event) {
    makeCurrent();
    doneCurrent();
    emit raceDone();
    event->accept();
}


void
RaceWindow::updateLabel(int iTeam, QString sLabel) {
    if((iTeam < 0) || (iTeam > 1)) return;
    sTeamName[iTeam] = sLabel;
    update();
}


void
RaceWindow::updateScore(int team0Score, int team1Score, int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    score[iSet].append(QVector2D(team0Score, team1Score));
    maxScore[iSet] = std::max(team0Score, team1Score);
}


void
RaceWindow::resetScore(int iSet) {
    if((iSet < 0) || (iSet > 4)) return;
    score[iSet].clear();
    score[iSet].append(QVector2D(0, 0));
    maxScore[iSet] = 0;
}


void
RaceWindow::resetAll() {
    for(int i=0; i<5; i++) {
        score[i].clear();
        score[i].append(QVector2D(0, 0));
        maxScore[i] = 0;
    }
}


void
RaceWindow::resizeGL(int w, int h) {
    cameraProjectionMatrix.setToIdentity();
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01f;
    const qreal zFar  = 30.0f;
    const qreal fov = 50.0;//abs(qRadiansToDegrees(atan2((xCamera-xField), (zCamera-zField))));
    cameraProjectionMatrix.perspective(fov, aspect, zNear, zFar);
}


void
RaceWindow::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    pFloor     = new Floor(50.0f, 50.0f);
    pPlayField = new PlayField(xField, zField);
    pTeam0     = new Sphere(ballRadius, 40, 40);
    pTeam1     = new Sphere(ballRadius, 40, 40);

    initEnvironment();
    initShaders();
    initTextures();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
}


void
RaceWindow::initShaders() {
/*
    pEnvironmentProgram = new QOpenGLShaderProgram();
    if(!pEnvironmentProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, environmentVShaderText)) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pEnvironmentProgram->log();
        delete pEnvironmentProgram;
        exit(EXIT_FAILURE);
    }
    if(!pEnvironmentProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, environmentFShaderText)) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pEnvironmentProgram->log();
        delete pEnvironmentProgram;
        exit(EXIT_FAILURE);
    }
    if (!pEnvironmentProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pEnvironmentProgram->log();
        delete pEnvironmentProgram;
        exit(EXIT_FAILURE);
    }
*/
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
RaceWindow::initTextures() {
    pTeam0Texture = new QOpenGLTexture(QImage(":/VolleyBall_0.png").mirrored());
    pTeam0Texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pTeam0Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam0Texture->setWrapMode(QOpenGLTexture::Repeat);

    pTeam1Texture = new QOpenGLTexture(QImage(":/VolleyBall_1.png").mirrored());
    pTeam1Texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pTeam1Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam1Texture->setWrapMode(QOpenGLTexture::Repeat);

    pFieldTexture = new QOpenGLTexture(QImage(":/blue-carpet.jpg").mirrored());
    pFieldTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pFieldTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pFieldTexture->setWrapMode(QOpenGLTexture::Repeat);

    pWoodTexture = new QOpenGLTexture(QImage(":/wood.png").mirrored());
    pWoodTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    pWoodTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pWoodTexture->setWrapMode(QOpenGLTexture::Repeat);

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
RaceWindow::startRace(int iSet) {
    if(maxScore[iSet] == 0) {
        return;
    }
    iCurrentSet = iSet;
    int totalPoints = score[iSet].count();
    pointTime  = (scanTime/totalPoints)*1000.0; // Tempo in ms per passare da un punto al successivo
    pointSpace = 2.0*xField/maxScore[iSet];     // Spazio da percorrere per ciascun punto.
    refreshTime = 12; // in ms
    float nTicks = pointTime/refreshTime;
    dx = pointSpace/nTicks;
    indexScore = 0;
    if(score[iCurrentSet].at(indexScore+1).x() > score[iCurrentSet].at(indexScore).x()) {
        dx0 = dx;
        dx1 = 0.0;
    }
    else {
        dx0 = 0.0;
        dx1 = dx;
    }
    x0  = x1  =-xField;
    t0 = 0.0;
//    qCritical() << score[iSet].at(indexScore+1);
    // Use QBasicTimer because its faster than QTimer
    timer.start(refreshTime, this);
}


void
RaceWindow::paintGL() {
    ConfigureModelMatrices();
/**/
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

// render Depth map to quad for visual debugging
// ---------------------------------------------
#ifdef SHOW_DEPTH
    pDebugDepthQuad->bind();
    pDebugDepthQuad->setUniformValue("near_plane", near_plane);
    pDebugDepthQuad->setUniformValue("far_plane",  far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderQuad();
#endif
/*
    pEnvironment->bind();
    pEnvironmentProgram->bind();
    pEnvironmentProgram->setUniformValue("mvp_matrix", projection * matrix);
    pEnvironmentProgram->setUniformValue("tex", GLint(0));
    pEnvironmentProgram->setUniformValue("env", GLint(1));
    pEnvironmentProgram->setUniformValue("noise", GLint(2));
    // m_box->draw();
    pEnvironmentProgram->release();
    pEnvironment->release();
*/

}


void
RaceWindow::ConfigureModelMatrices() {
    fieldModelMatrix.setToIdentity();
    fieldModelMatrix.scale(1.0f, 1.0f, 1.0f);
    fieldModelMatrix.translate(0.0f, 0.0f, 0.0f);

    float angle = qRadiansToDegrees(dx0/ballRadius);
    x0 += dx0;
    if(std::abs(x0) > xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    rotation0 = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0,-1.0), angle) * rotation0;
    team0ModelMatrix.setToIdentity();
    team0ModelMatrix.translate(x0, ballRadius, z0Start);
    team0ModelMatrix.rotate(rotation0);

    angle = qRadiansToDegrees(dx1/ballRadius);
    x1 += dx1;
    if(std::abs(x1) > xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    rotation1 = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0,-1.0), angle) * rotation1;
    team1ModelMatrix.setToIdentity();
    team1ModelMatrix.translate(x1, ballRadius, z1Start);
    team1ModelMatrix.rotate(rotation1);

    floorModelMatrix.setToIdentity();
    floorModelMatrix.translate(0.0f, -0.01f, 0.0f);
}


void
RaceWindow::renderScene(QOpenGLShaderProgram* pProgram) {
    glActiveTexture(GL_TEXTURE0);

    pProgram->setUniformValue("model", floorModelMatrix);
    pWoodTexture->bind();
    pFloor->draw(pProgram);

    pProgram->setUniformValue("model", fieldModelMatrix);
    pFieldTexture->bind();
    pPlayField->draw(pProgram);

    pProgram->setUniformValue("model", team0ModelMatrix);
    pTeam0Texture->bind();
    pTeam0->draw(pProgram);

    pProgram->setUniformValue("model", team1ModelMatrix);
    pTeam1Texture->bind();
    pTeam1->draw(pProgram);

}


void
RaceWindow::initEnvironment() {
    pEnvironment = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
    pEnvironment->setWrapMode(QOpenGLTexture::ClampToEdge);
    pEnvironment->setMinificationFilter(QOpenGLTexture::Linear);
    pEnvironment->setMagnificationFilter(QOpenGLTexture::Linear);
    pEnvironment->setMipBaseLevel(0);
    pEnvironment->setMipMaxLevel(0);
    QImage* pImage = new QImage(":/res/cubemap_posx.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->allocateStorage();
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapPositiveX,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;
    pImage = new QImage(":/res/cubemap_negx.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapNegativeX,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;
    pImage = new QImage(":/res/cubemap_posy.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapPositiveY,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;
    pImage = new QImage(":/res/cubemap_negy.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapNegativeY,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;

    pImage = new QImage(":/res/cubemap_posz.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapPositiveZ,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;
    pImage = new QImage(":/res/cubemap_negz.jpg");
    pImage->convertTo(QImage::Format_RGBA8888);
    pEnvironment->setData(0,
                          0,
                          QOpenGLTexture::CubeMapNegativeZ,
                          QOpenGLTexture::RGBA,
                          QOpenGLTexture::UInt32,
                          pImage->data_ptr(),
                          nullptr);
    delete pImage;
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void
RaceWindow::renderQuad() {
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
RaceWindow::timerEvent(QTimerEvent*) {
    t0 += refreshTime;
    if(t0 > pointTime) {
        t0 = 0.0;
        indexScore++;
        if(indexScore >= score[iCurrentSet].count()-1) {
            timer.stop();
            closeTimer.start(3000);
            update();
            return;
        }
        if(score[iCurrentSet].at(indexScore+1).x() > score[iCurrentSet].at(indexScore).x()) {
            dx0 = dx;
            dx1 = 0.0;
        }
        else {
            dx0 = 0.0;
            dx1 = dx;
        }
    }
    update();
}


void
RaceWindow::onTimeToClose() {
    closeTimer.stop();
    emit raceDone();
}
