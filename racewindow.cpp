#include "racewindow.h"
#include "sphere.h"
#include "playfield.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>


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
RaceWindow::resizeGL(int w, int h) {
    viewMatrix.setToIdentity();
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 0.01f, zFar = 18.0f, fov = 65.0f;
    viewMatrix.perspective(fov, aspect, zNear, zFar);
}


void
RaceWindow::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    diffuseColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

    ballRadius = 0.1066f*5.0f; // Five times bigger than real

    pTeam0     = new Sphere(ballRadius, 40, 40);
    pTeam1     = new Sphere(ballRadius, 40, 40);
    pPlayField = new PlayField();

    xCamera =  0.0;
    yCamera = 10.0;
    zCamera = 10.0;

    cameraMatrix.lookAt(QVector3D(xCamera, yCamera, zCamera),  // Eye
                        QVector3D(0.0f, 0.0f, 0.0f),  // Center
                        QVector3D(0.0f, 1.0f, 0.0f)); // Up

    initEnvironment();
    initShaders();
    initTextures();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Use QBasicTimer because its faster than QTimer
    dx = 0.0;
    x0 = -xField;
    timer.start(12, this);
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
    pTeam0Texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pTeam0Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pTeam0Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam0Texture->setWrapMode(QOpenGLTexture::Repeat);

    pTeam1Texture = new QOpenGLTexture(QImage(":/earth.png").mirrored());
    pTeam1Texture->setMinificationFilter(QOpenGLTexture::Nearest);
    pTeam1Texture->setMagnificationFilter(QOpenGLTexture::Linear);
    pTeam1Texture->setWrapMode(QOpenGLTexture::Repeat);

    pFieldTexture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    pFieldTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    pFieldTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pFieldTexture->setWrapMode(QOpenGLTexture::Repeat);
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
    modelMatrix.translate(0.0f, 0.0f, 0.0f);
    modelMatrix.scale(xField, 0.1f, zField);
    modelViewMatrix = cameraMatrix * modelMatrix;
    pGameProgram->setUniformValue("model",        modelMatrix);
    pGameProgram->setUniformValue("modelView",    modelViewMatrix);
    pGameProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());
    pFieldTexture->bind();
    pPlayField->draw(pGameProgram);

    float angle = (dx/ballRadius)*180.0/M_PI;
    x0 += dx;
    rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0,-1.0), angle) * rotation;
    modelMatrix.setToIdentity();
    modelMatrix.translate(x0, ballRadius, z0Start);
    modelMatrix.rotate(rotation);
    modelViewMatrix = cameraMatrix * modelMatrix;
    pGameProgram->setUniformValue("model",        modelMatrix);
    pGameProgram->setUniformValue("modelView",    modelViewMatrix);
    pGameProgram->setUniformValue("normalMatrix", modelViewMatrix.normalMatrix());

    pTeam0Texture->bind();
    pTeam0->draw(pGameProgram);

    modelMatrix.setToIdentity();
    modelMatrix.translate(-xField, ballRadius, z1Start);
    modelMatrix.translate(0.0f, 0.0f, 0.0f);
    modelMatrix.rotate(rotation);
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
    dx = 0.012;
    update();
}

