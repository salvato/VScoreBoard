#include "racewindow.h"
#include "TeamAvatar.h"
#include "sphere.h"
#include "playfield.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>



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

    lightPosition = QVector4D(0.0, 20.0, 4.0, 1.0);
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
RaceWindow::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    pTeam0     = new TeamAvatar();
    pTeam1     = new TeamAvatar();
    pPlayField = new PlayField();
    pSphere    = new Sphere(1.0f, 40, 40);

    cameraMatrix.lookAt(QVector3D(0.0f, 0.0f, 3.0f),  // Eye
                        QVector3D(0.0f, 0.0f, 0.0f),  // Center
                        QVector3D(0.0f, 1.0f, 0.0f)); // Up

    initEnvironment();
//    initPlayField();

    initShaders();
    initTextures();

/*
    if((currentAnimation >= pPrograms.count()) ||
       (currentAnimation < 0))
        currentAnimation = 0;
    pCurrentProgram = pPrograms.at(currentAnimation);
    if(!pCurrentProgram->bind()) {
        qCritical() << __FUNCTION__ << __LINE__;
        close();
        return;
    }
    getLocations();
*/

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}


void
RaceWindow::initShaders() {
    pFieldProgram = new QOpenGLShaderProgram();
    if (!pFieldProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vShader.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }
    if (!pFieldProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fAvatar.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }
    if (!pFieldProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }

    pAvatarProgram = new QOpenGLShaderProgram();
    if (!pAvatarProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vShader.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pAvatarProgram->log();
        delete pAvatarProgram;
        exit(EXIT_FAILURE);
    }
    if (!pAvatarProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fAvatar.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pAvatarProgram->log();
        delete pAvatarProgram;
        exit(EXIT_FAILURE);
    }
    if (!pAvatarProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pAvatarProgram->log();
        delete pAvatarProgram;
        exit(EXIT_FAILURE);
    }

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

    pSphereProgram = new QOpenGLShaderProgram();
    if(!pSphereProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vSphere.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pSphereProgram->log();
        delete pSphereProgram;
        exit(EXIT_FAILURE);
    }
    if(!pSphereProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fSphere.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pSphereProgram->log();
        delete pSphereProgram;
        exit(EXIT_FAILURE);
    }
    if (!pSphereProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pSphereProgram->log();
        delete pSphereProgram;
        exit(EXIT_FAILURE);
    }
}


void
RaceWindow::initTextures() {
    pAvatar0Texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pAvatar0Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pAvatar0Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pAvatar0Texture->setWrapMode(QOpenGLTexture::Repeat);

    pAvatar1Texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pAvatar1Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pAvatar1Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pAvatar1Texture->setWrapMode(QOpenGLTexture::Repeat);

    pFieldTexture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pFieldTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    pFieldTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pFieldTexture->setWrapMode(QOpenGLTexture::Repeat);

    pSphereTexture = new QOpenGLTexture(QImage(":/earth.png").mirrored());
    pSphereTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    pSphereTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pSphereTexture->setWrapMode(QOpenGLTexture::Repeat);
}


void
RaceWindow::resizeGL(int w, int h) {
    viewMatrix.setToIdentity();
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01f, zFar = 18.0f, fov = 65.0f;
    viewMatrix.perspective(fov, aspect, zNear, zFar);
}


void
RaceWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pSphereProgram->bind();
    pSphereTexture->bind();

    modelMatrix.setToIdentity();
    //modelMatrix.translate(xField, 1.0f, z0Start);
    modelMatrix.translate(0.0f, 0.0f, 0.0f);
    modelMatrix.rotate(rotation);

    pSphereProgram->setUniformValue("camera",   cameraMatrix);
    pSphereProgram->setUniformValue("model",    modelMatrix);
    pSphereProgram->setUniformValue("view",     viewMatrix);
    pSphereProgram->setUniformValue("Tex0",     0);
    pSphereProgram->setUniformValue("lightPos", lightPosition);

    QMatrix4x4 modelViewMatrix = cameraMatrix * modelMatrix;
    pSphereProgram->setUniformValue("modelView",    modelViewMatrix);
    pSphereProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());

    pSphere->draw(pSphereProgram);

    pFieldTexture->bind();

//    modelMatrix.setToIdentity();
//    modelMatrix.translate(0.0f, 0.0f, 0.0f);
//    modelMatrix.scale(xField, 0.1f, zField);

    pPlayField->draw(pSphereProgram);

/*
    pAvatarProgram->bind();
    pAvatar0Texture->bind();
    modelMatrix.setToIdentity();
    //modelMatrix.translate(xField, 1.0, z0Start);
    modelMatrix.translate(0.0, 0.0, -5.0);
    modelMatrix.rotate(rotation);

    pAvatarProgram->setUniformValue("mvp_matrix", projectionMatrix * viewMatrix * modelMatrix);
    pAvatarProgram->setUniformValue("texture", 0);
    pTeam0->draw(pAvatarProgram);
*/


/*
    modelMatrix.setToIdentity();
    modelMatrix.translate(xField, 1.0, z1Start);
    //modelMatrix.rotate(rotation);

    pAvatarProgram->setUniformValue("mvp_matrix", projectionMatrix * viewMatrix * modelMatrix);
    pAvatar1Texture->bind();
    pAvatarProgram->setUniformValue("texture", 0);
    pTeam1->drawAvatar(pAvatarProgram);
*/

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
RaceWindow::mousePressEvent(QMouseEvent* pEvent) {
    // Save mouse press position
    mousePressPosition = QVector2D(pEvent->position());
}

void
RaceWindow::mouseReleaseEvent(QMouseEvent* pEvent) {
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(pEvent->position()) - mousePressPosition;
    // Rotation axis perpendicular to the mouse position difference vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;
    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();
    // Increase angular speed
    angularSpeed += acc;
}


void
RaceWindow::wheelEvent(QWheelEvent* pEvent) {
    QOpenGLWidget::wheelEvent(pEvent);
    if(!pEvent->isAccepted()) {
        zCamera += pEvent->angleDelta().y()/(8.0*360.0);
//        if(zCamera < -8 * 120)
//            zCamera = -8 * 120;
//        if(zCamera > 10 * 120)
//            zCamera = 10 * 120;
        update();
        pEvent->accept();
    }
}


void
RaceWindow::timerEvent(QTimerEvent *) {
    // Decrease angular speed (friction)
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    }
    else {// Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
        // Request an update
        update();
    }
}

