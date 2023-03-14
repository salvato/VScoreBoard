#include "slidewidget.h"

#include <QMouseEvent>
#include <QDir>
#include <cmath>
#include <QApplication>
#include <QPainter>
#include <QStandardPaths>


#define STEADY_SHOW_TIME       3000 // Change slide time
#define TRANSITION_TIME        1500 // Transition duration
#define UPDATE_TIME              30 // Time between screen updates


SlideWidget::SlideWidget()
    : QOpenGLWidget()
    , arrayBuf(QOpenGLBuffer::VertexBuffer)
    , bRunning(false)
    , pBaseImage(nullptr)
{
    srand(QTime::currentTime().msec());
    sSlideDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    updateSlideList();

    QList<QScreen*> screens = QApplication::screens();
    QRect screenres = screens.at(0)->geometry();
    if(screens.count() > 1) {
        screenres = screens.at(1)->geometry();
        QPoint point = QPoint(screenres.x(), screenres.y());
        move(point);
    }
    pBaseImage = new QImage(screenres.width(),
                            screenres.height(),
                            QImage::Format_RGBA8888_Premultiplied);

    // Reset projection matrix
    projection.setToIdentity();
    viewingDistance       = 20.0f;
    aspectRatio           = GLfloat(screenres.width())/GLfloat(screenres.height());
    GLfloat verticalAngle = 2.0*atan(1.0/viewingDistance)*180.0/M_PI;
    GLfloat nearPlane     = viewingDistance - 1.0;
    GLfloat farPlane      = viewingDistance + 1.0;
    projection.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);

    iCurrentSlide    = 0;

    timerSteady.setSingleShot(true);
    connect(&timerAnimate, SIGNAL(timeout()),
            this, SLOT(ontimerAnimateEvent()));
    connect(&timerSteady, SIGNAL(timeout()),
            this, SLOT(onTimerSteadyEvent()));
}


SlideWidget::~SlideWidget() {
    makeCurrent();
    if(pTexture0) delete pTexture0;
    pTexture0 = nullptr;
    if(pTexture1) delete pTexture1;
    pTexture1 = nullptr;
    doneCurrent();
}


void
SlideWidget::closeEvent(QCloseEvent* event) {
    makeCurrent();
    if(pTexture0) delete pTexture0;
    pTexture0 = nullptr;
    if(pTexture1) delete pTexture1;
    pTexture1 = nullptr;
    doneCurrent();
    event->accept();
}


bool
SlideWidget::setSlideDir(QString sNewDir) {
    if(sNewDir != sSlideDir) {
        if(timerSteady.isActive())  timerSteady.stop();
        if(timerAnimate.isActive()) timerAnimate.stop();
        sSlideDir = sNewDir;
        if(!updateSlideList())
            return false;
        iCurrentSlide = 0;
    }
    if(bRunning)
        return startSlideShow();
    else
        return true;
}


bool
SlideWidget::startSlideShow() {
//    currentAnimation = nAnimationTypes-1;
    currentAnimation = rand() % nAnimationTypes;
    makeCurrent();
    pCurrentProgram = pPrograms.at(currentAnimation);
    if(!pCurrentProgram->bind()) {
        qCritical() << __FUNCTION__ << __LINE__;
        close();
        return false;
    }
    getLocations();
    doneCurrent();
    setWindowTitle(pCurrentProgram->objectName());
    update();
    timerSteady.start(STEADY_SHOW_TIME);
    bRunning = true;
    return true;
}


void
SlideWidget::stopSlideShow() {
    timerSteady.stop();
    timerAnimate.stop();
    bRunning = false;
}


bool
SlideWidget::updateSlideList() {
    // Update slide list just in case we are updating the slide directory...
    slideList = QFileInfoList();
    QDir slideDir(sSlideDir);
    if(slideDir.exists()) {
        QStringList nameFilter = QStringList() << "*.jpg" << "*.jpeg" << "*.png";
        slideDir.setNameFilters(nameFilter);
        slideDir.setFilter(QDir::Files);
        slideList = slideDir.entryInfoList();
    }
    return !slideList.isEmpty();
}


bool
SlideWidget::prepareNextSlide() {
    QImage newImage(slideList.at(iCurrentSlide).absoluteFilePath());
    image = newImage.scaled(pBaseImage->size(),
                            Qt::KeepAspectRatio).mirrored();
    QPainter painter(pBaseImage);
    painter.fillRect(pBaseImage->rect(), Qt::white);
    int x = (pBaseImage->width()  - image.width())  / 2;
    int y = (pBaseImage->height() - image.height()) / 2;
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(x, y, image);
    painter.end();

    iCurrentSlide = (iCurrentSlide + 1) % slideList.count();
    return true;
}


bool
SlideWidget::prepareNextRound() {
    makeCurrent(); // Fondamentale !!!
//    currentAnimation = nAnimationTypes-1;
    currentAnimation = rand() % nAnimationTypes;
    pCurrentProgram->release();
    pCurrentProgram = pPrograms.at(currentAnimation);
    if(!pCurrentProgram->bind()) {
        qCritical() << __FUNCTION__ << __LINE__;
        close();
        return false;
    }
    getLocations();

    // Prepare the next slide...
    if(!prepareNextSlide()) {
        close();
        return false;
    }
    pTexture0->release();
    delete pTexture0;
    pTexture1->release();
    pTexture0 = pTexture1;
    pTexture1 = new QOpenGLTexture(*pBaseImage);
    pTexture1->setMinificationFilter(QOpenGLTexture::Nearest);
    pTexture1->setMagnificationFilter(QOpenGLTexture::Linear);
    pTexture1->setWrapMode(QOpenGLTexture::Repeat);
    doneCurrent();
    setWindowTitle(pCurrentProgram->objectName());
    timerSteady.start(STEADY_SHOW_TIME);
    return true;
}


bool
SlideWidget::getLocations() {
    if(!pCurrentProgram->isLinked()) {
        qCritical() << "Program not linked";
        close();
        return false;
    }
    int vertexLocation   = pCurrentProgram->attributeLocation("a_position");
    int texcoordLocation = pCurrentProgram->attributeLocation("a_texcoord");
    if((vertexLocation   == -1) || (texcoordLocation == -1)) {
        qCritical() << "Shader attributes not found";
        close();
        return false;
    }

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    pCurrentProgram->enableAttributeArray(vertexLocation);
    pCurrentProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    pCurrentProgram->enableAttributeArray(texcoordLocation);
    pCurrentProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    iTex0Loc = pCurrentProgram->uniformLocation("texture0");
    iTex1Loc = pCurrentProgram->uniformLocation("texture1");

    iProgressLoc = pCurrentProgram->uniformLocation("progress");
    if(iProgressLoc == -1) {
        qCritical() << __FUNCTION__ << __LINE__ << "Shader uniform not found";
        close();
        return false;
    }
    progress = 0.0;

    return true;
}


void
SlideWidget::drawGeometry(QOpenGLShaderProgram *program) {
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices);
}


void
SlideWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

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
}


void
SlideWidget::initShaders() {
    QStringList fShaderList = QStringList({"fBookFlip",
                                          "fAngular",
                                          "fBounce",
                                          "fFilmBurn",
                                          "fDoomScreen",
                                          "fWaterDrop",
                                          "fFlyEye",
                                          "fMorph",
                                          "fPerlin",
                                          "fPinwheel",
                                          "fPolkaDotsCurtain",
                                          "fPowerKaleido",
                                          "fInvertedPageCurl",
                                          "fDisplacement",
                                          "fSwirl",
                                          "fDreamy",
                                          "fCrosshatch",
                                          "fRadial",
                                          "fRipple",
                                          "fCircleopen",
                                          "fFade",
                                          "fMultiply_blend",
                                          "fPixelize",
                                          "fWind",
                                          "fSwap",
                                          "fCrosswarp"});
    for(int i=0; i<fShaderList.count(); i++) {
        QOpenGLShaderProgram* pNewProgram = new QOpenGLShaderProgram(this);
        if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vShader.glsl")) {
            close();
            return;
        }
        QString sFshader = QString(":/Shaders/%1.glsl").arg(fShaderList.at(i));
        if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, sFshader)) {
            close();
            return;
        }
        if (!pNewProgram->link()) {
            close();
            return;
        }
        pNewProgram->setObjectName(fShaderList.at(i));
        pPrograms.append(pNewProgram);
    }

    nAnimationTypes = pPrograms.count();
    currentAnimation = rand() % nAnimationTypes;
}


void
SlideWidget::initTextures() {
    // Setup the first texture
    if(!prepareNextSlide()) {
        close();
        return;
    }
    pTexture0 = new QOpenGLTexture(*pBaseImage);
    pTexture0->setMinificationFilter(QOpenGLTexture::Nearest);
    pTexture0->setMagnificationFilter(QOpenGLTexture::Linear);
    pTexture0->setWrapMode(QOpenGLTexture::Repeat);
    // Now the second texture
    if(!prepareNextSlide()) {
        close();
        return;
    }
    pTexture1 = new QOpenGLTexture(*pBaseImage);
    pTexture1->setMinificationFilter(QOpenGLTexture::Nearest);
    pTexture1->setMagnificationFilter(QOpenGLTexture::Linear);
    pTexture1->setWrapMode(QOpenGLTexture::Repeat);
}


void
SlideWidget::initGeometry() {
    QVector<VertexData> vertices;
    int nxStep = 54;
    int nyStep = 36;
    float dx = 2.0/nxStep;
    float dy = 2.0/nyStep;
    float xdx, ydy;

    float x, y, xT, yT;
    x = -1.0;
    for(int i=0; i<nxStep; i++) {
        xdx = x + dx;
        if(xdx >= 1.0f) xdx= 1.0f;
        xT = x + 1.0;
        y= -1.0f;
        for(int j=0; j<nyStep; j++) {
            ydy = y + dy;
            if(ydy >= 1.0f) ydy = 1.0f;
            yT = y + 1.0;
            vertices.append({QVector3D(x*aspectRatio,   y,   0.0f),
                             QVector2D(0.5*xT,        0.5*yT)});
            vertices.append({QVector3D(xdx*aspectRatio, y,   0.0f),
                             QVector2D(0.5*(xdx+1.0), 0.5*yT)});
            vertices.append({QVector3D(x*aspectRatio,   ydy, 0.0f),
                             QVector2D(0.5*xT,        0.5*(ydy+1.0))});
            vertices.append({QVector3D(xdx*aspectRatio, ydy, 0.0f),
                             QVector2D(0.5*(xdx+1.0), 0.5*(ydy+1.0))});
            y = ydy;
        }
        x = xdx;
    }
    nVertices = vertices.count();
    // Transfer vertex data to VBO
    arrayBuf.create();
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), nVertices*sizeof(VertexData));
}


void
SlideWidget::resizeGL(int w, int h) {
    Q_UNUSED(w)
    Q_UNUSED(h)
}


void
SlideWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepthf(5.0f);

    pCurrentProgram->setUniformValue(iTex0Loc, 0);
    pCurrentProgram->setUniformValue(iTex1Loc, 1);
    pTexture0->bind(0);
    pTexture1->bind(1);
    pCurrentProgram->setUniformValue(iProgressLoc, progress);
    matrix.setToIdentity();
    matrix.translate(0.0f, 0.0f, -viewingDistance+0.01);
    // Set modelview-projection matrix
    pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
    drawGeometry(pCurrentProgram);

    glDisable(GL_DEPTH_TEST);
}


void
SlideWidget::ontimerAnimateEvent() {
    progress += 0.02f;
    if(progress >= 1.0f) {
        timerAnimate.stop();
        prepareNextRound();
        progress = 0.0f;
    }

    update();
}


void
SlideWidget::onTimerSteadyEvent() {
    timerAnimate.start(UPDATE_TIME);
}

