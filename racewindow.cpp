#include "racewindow.h"
#include "TeamAvatar.h"

#include <QApplication>
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
    QList<QScreen*> screens = QApplication::screens();
    QRect screenres = screens.at(0)->geometry();
    if(screens.count() > 1) {
        screenres = screens.at(1)->geometry();
        QPoint point = QPoint(screenres.x(), screenres.y());
        move(point);
    }

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
    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    pTeam0 = new TeamAvatar();
    pTeam1 = new TeamAvatar();

    initEnvironment();
    initPlayField();

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
    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}


void
RaceWindow::initShaders() {
    pFieldProgram = new QOpenGLShaderProgram();
    // Compile Avatar vertex shader
    if (!pFieldProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vShader.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }
    // Compile Avatar fragment shader
    if (!pFieldProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fAvatar.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }
    // Link Avatat shader pipeline
    if (!pFieldProgram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << pFieldProgram->log();
        delete pFieldProgram;
        exit(EXIT_FAILURE);
    }

    pAvatarProgram = new QOpenGLShaderProgram();
    // Compile Avatar vertex shader
    if (!pAvatarProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vShader.glsl")) {
        qWarning("Failed to compile vertex shader program");
        qWarning("Shader program log:");
        qWarning() << pAvatarProgram->log();
        delete pAvatarProgram;
        exit(EXIT_FAILURE);
    }
    // Compile Avatar fragment shader
    if (!pAvatarProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fAvatar.glsl")) {
        qWarning("Failed to compile fragment shader program");
        qWarning("Shader program log:");
        qWarning() << pAvatarProgram->log();
        delete pAvatarProgram;
        exit(EXIT_FAILURE);
    }
    // Link Avatat shader pipeline
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
}


void
RaceWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepthf(5.0f);

    pAvatarProgram->bind();

    QMatrix4x4 modelMatrix;

    modelMatrix.setToIdentity();
    modelMatrix.translate(0.0, 1.0, 0.0);
    modelMatrix.scale(9.0, 1.0, 4.5);
    //modelMatrix.rotate(rotation);

    pFieldProgram->setUniformValue("mvp_matrix", projectionMatrix * viewMatrix * modelMatrix);
    pFieldTexture->bind();
    pFieldProgram->setUniformValue("texture", 0);
    drawField(pAvatarProgram);

    modelMatrix.setToIdentity();
    modelMatrix.translate(xField, 1.0, z0Start);
    //modelMatrix.rotate(rotation);

    pAvatarProgram->setUniformValue("mvp_matrix", projectionMatrix * viewMatrix * modelMatrix);
    pAvatar0Texture->bind();
    pAvatarProgram->setUniformValue("texture", 0);
    pTeam0->drawAvatar(pAvatarProgram);

    modelMatrix.setToIdentity();
    modelMatrix.translate(xField, 1.0, z1Start);
    //modelMatrix.rotate(rotation);

    pAvatarProgram->setUniformValue("mvp_matrix", projectionMatrix * viewMatrix * modelMatrix);
    pAvatar1Texture->bind();
    pAvatarProgram->setUniformValue("texture", 0);
    pTeam1->drawAvatar(pAvatarProgram);

//    pEnvironment->bind();
//    pEnvironmentProgram->bind();
//    pEnvironmentProgram->setUniformValue("mvp_matrix", projection * matrix);
//    pEnvironmentProgram->setUniformValue("tex", GLint(0));
//    pEnvironmentProgram->setUniformValue("env", GLint(1));
//    pEnvironmentProgram->setUniformValue("noise", GLint(2));
//    // m_box->draw();
//    pEnvironmentProgram->release();
//    pEnvironment->release();

    glDisable(GL_DEPTH_TEST);
}


void
RaceWindow::resizeGL(int w, int h) {
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.0, zFar = 7.0, fov = 25.0;
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(fov, aspect, zNear, zFar);
    viewMatrix.setToIdentity();
    viewMatrix.lookAt(QVector3D(xCamera, yCamera, zCamera), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));
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


/*
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
//Define all 6 faces
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face0);
glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face1);
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face2);
glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face3);
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face4);
glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face5);
*/
