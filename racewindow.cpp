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
#include "sphere.h"
#include "playfield.h"

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

    ballRadius = 0.1066f*5.0f; // Five times bigger than real

    diffuseColor  = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    lightPosition = QVector4D(0.0, 20.0, 4.0, 1.0);

    xCamera =  0.0f;
    yCamera = 10.0f;
    zCamera = 10.0f;
    cameraMatrix.lookAt(QVector3D(xCamera, yCamera, zCamera), // Eye
                        QVector3D(0.0f,    0.0f,    0.0f),    // Center
                        QVector3D(0.0f,    1.0f,    0.0f));   // Up

    resetAll();
    scanTime = 10.0; // Tempo in secondi per l'intera "Corsa"
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
    viewMatrix.setToIdentity();
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01f, zFar = 18.0f;
    const qreal fov = 50.0;//abs(qRadiansToDegrees(atan2((xCamera-xField), (zCamera-zField))));
//    qCritical() << "fov" << fov;
    viewMatrix.perspective(fov, aspect, zNear, zFar);
}


void
RaceWindow::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    pTeam0     = new Sphere(ballRadius, 40, 40);
    pTeam1     = new Sphere(ballRadius, 40, 40);
    pPlayField = new PlayField();


    initEnvironment();
    initShaders();
    initTextures();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
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
    if(!pGameProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vSphere.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pGameProgram->log();
        delete pGameProgram;
        exit(EXIT_FAILURE);
    }
    if(!pGameProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fSphere.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pGameProgram->log();
        delete pGameProgram;
        exit(EXIT_FAILURE);
    }
    if (!pGameProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pGameProgram->log();
        delete pGameProgram;
        exit(EXIT_FAILURE);
    }
}


void
RaceWindow::initTextures() {
    pTeam0Texture = new QOpenGLTexture(QImage(":/VolleyBall_0.png").mirrored());
    pTeam0Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pTeam0Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam0Texture->setWrapMode(QOpenGLTexture::Repeat);

    pTeam1Texture = new QOpenGLTexture(QImage(":/VolleyBall_1.png").mirrored());
    pTeam1Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pTeam1Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam1Texture->setWrapMode(QOpenGLTexture::Repeat);

    pFieldTexture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pFieldTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    pFieldTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pFieldTexture->setWrapMode(QOpenGLTexture::Repeat);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameProgram->bind();
    pGameProgram->setUniformValue("camera",   cameraMatrix);
    pGameProgram->setUniformValue("view",     viewMatrix);
    pGameProgram->setUniformValue("Tex0",     0);
    pGameProgram->setUniformValue("lightPos", lightPosition);
    pGameProgram->setUniformValue("vColor",   diffuseColor);
    pGameProgram->setUniformValue("vSColor",  specularColor);

    modelMatrix.setToIdentity();
    modelMatrix.scale(xField, 0.1f, zField);
    modelMatrix.translate(0.0f, -1.0f, 0.0f);
    modelViewMatrix = cameraMatrix * modelMatrix;
    pGameProgram->setUniformValue("model",        modelMatrix);
    pGameProgram->setUniformValue("modelView",    modelViewMatrix);
    pGameProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());
    pFieldTexture->bind();
    pPlayField->draw(pGameProgram);

    float angle = qRadiansToDegrees(dx0/ballRadius);
    x0 += dx0;
    if(x0 > xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    if(x0 < -xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    rotation0 = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0,-1.0), angle) * rotation0;
    modelMatrix.setToIdentity();
    modelMatrix.translate(x0, ballRadius, z0Start);
    modelMatrix.rotate(rotation0);
    modelViewMatrix = cameraMatrix * modelMatrix;
    pGameProgram->setUniformValue("model",        modelMatrix);
    pGameProgram->setUniformValue("modelView",    modelViewMatrix);
    pGameProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());
    pTeam0Texture->bind();
    pTeam0->draw(pGameProgram);

    angle = qRadiansToDegrees(dx1/ballRadius);
    x1 += dx1;
    if(x1 > xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    if(x1 <-xField) {
        timer.stop();
        closeTimer.start(3000);
    }
    rotation1 = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0,-1.0), angle) * rotation1;
    modelMatrix.setToIdentity();
    modelMatrix.translate(x1, ballRadius, z1Start);
    modelMatrix.rotate(rotation1);
    modelViewMatrix = cameraMatrix * modelMatrix;
    pGameProgram->setUniformValue("model",        modelMatrix);
    pGameProgram->setUniformValue("modelView",    modelViewMatrix);
    pGameProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());
    pTeam1Texture->bind();
    pTeam1->draw(pGameProgram);

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

//    glDisable(GL_DEPTH_TEST);
}


void
RaceWindow::initPlayField() {
    QVector<VertexData> vertices;
    vertices.append({QVector3D( 1.0, 0.0,  1.0f), QVector2D(1.0, 1.0)});
    vertices.append({QVector3D(-1.0, 0.0,  1.0f), QVector2D(0.0, 1.0)});
    vertices.append({QVector3D(-1.0, 0.0, -1.0f), QVector2D(0.0, 0.0)});

    vertices.append({QVector3D( 1.0, 0.0,  1.0f), QVector2D(1.0, 1.0)});
    vertices.append({QVector3D( 1.0, 0.0, -1.0f), QVector2D(1.0, 0.0)});
    vertices.append({QVector3D(-1.0, 0.0, -1.0f), QVector2D(0.0, 0.0)});

    nVertices = vertices.count();

    // Transfer vertex data to VBO
    fieldBuf.create();
    fieldBuf.bind();
    fieldBuf.allocate(vertices.data(), nVertices*sizeof(VertexData));
    fieldBuf.release();
}


void
RaceWindow::drawField(QOpenGLShaderProgram* pProgram) {
    // Tell OpenGL which VBOs to use
    fieldBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = pProgram->attributeLocation("a_position");
    pProgram->enableAttributeArray(vertexLocation);
    pProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = pProgram->attributeLocation("a_texcoord");
    pProgram->enableAttributeArray(texcoordLocation);
    pProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices);
}


void
RaceWindow::initEnvironment() {
//    m_box = new GLRoundedBox(0.25f, 1.0f, 10);
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
