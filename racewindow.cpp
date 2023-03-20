#include "racewindow.h"

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

    initEnvironment();

/*
    initShaders();
    initTextures();
    initGeometry();

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
}


void
RaceWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepthf(5.0f);

    glDisable(GL_DEPTH_TEST);
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
