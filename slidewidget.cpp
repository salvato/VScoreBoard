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
    if(screens.count() > 1) {
        QRect screenres = screens.at(1)->geometry();
        QPoint point = QPoint(screenres.x(), screenres.y());
        move(point);
    }

    iCurrentSlide    = 0;

    A      = A0      = QVector4D(0.0f, -1.0f, 0.0f, 1.0f);
    theta  = theta0  = GLfloat(M_PI_2);
    angle  = angle0  = 0.0f;
    alpha  = alpha0  = 1.0f;
    fScale = fScale0 = 1.0f;
    fRot   = fRot0   = 0.0f;
    xLeft  =-GLfloat(width())/GLfloat(height());

    viewingDistance  = 20.0f;

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
    if(slideList.count() == 0) {
        qCritical() << "No Slides in directory: exiting ...";
        close();
        return false;
    }
    if(iCurrentSlide >= slideList.count()) {
        iCurrentSlide = iCurrentSlide % slideList.count();
        qCritical() << "Errore: iCurrentSlide >= slideList.count()";
    }
    QImage newImage(slideList.at(iCurrentSlide).absoluteFilePath());
    image = newImage.scaled(pBaseImage->width(), pBaseImage->height(), Qt::KeepAspectRatio).mirrored();

    QPainter painter(pBaseImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, pBaseImage->width(), pBaseImage->height(), Qt::white);
    int x = (pBaseImage->width()-image.width())/2;
    int y = (pBaseImage->height()-image.height())/2;
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(x, y, image);
    painter.end();

    iCurrentSlide = (iCurrentSlide + 1) % slideList.count();
    return true;
}


bool
SlideWidget::prepareNextRound() {
    makeCurrent(); // Fondamentale !!!
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

    if(currentAnimation == 0) {// Fold effect
        iALoc     = pCurrentProgram->uniformLocation("a");
        iThetaLoc = pCurrentProgram->uniformLocation("theta");
        iAngleLoc = pCurrentProgram->uniformLocation("angle");
        iLeftLoc  = pCurrentProgram->uniformLocation("xLeft");
        if((iALoc     == -1) || (iThetaLoc == -1) ||
           (iAngleLoc == -1) || (iLeftLoc  == -1))
        {
            qCritical() << __FUNCTION__ << __LINE__ << "Shader uniforms not found";
            close();
            return false;
        }
    } // currentAnimation == 0: Fold effect

    else if(currentAnimation == 1) {// Fade effect
        iTex1Loc = pCurrentProgram->uniformLocation("texture1");
        iAlphaLoc = pCurrentProgram->uniformLocation("alpha");
        if(iAlphaLoc == -1) {
            qCritical() << __FUNCTION__ << __LINE__ << "alpha uniform not found";
            close();
            return false;
        }
    } // currentAnimation == 1: Fade effect

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
    if(pBaseImage) delete pBaseImage;
    pBaseImage = new QImage(width(),
                            height(),
                            QImage::Format_RGBA8888_Premultiplied);
    initShaders();
    initTextures();
    initGeometry();

    currentAnimation = 1;
    pCurrentProgram = pPrograms.at(currentAnimation);
    if(!pCurrentProgram->bind()) {
        qCritical() << __FUNCTION__ << __LINE__;
        close();
        return;
    }
    getLocations();

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}


void
SlideWidget::initShaders() {
    QOpenGLShaderProgram* pNewProgram = new QOpenGLShaderProgram(this);
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshaderFold.glsl")) {
        close();
        return;
    }

    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshaderFold.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->link()) {
        close();
        return;
    }
    pNewProgram->setObjectName("Fold");
    pPrograms.append(pNewProgram);// Fold page effect at 0

    pNewProgram = new QOpenGLShaderProgram(this);
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshaderFade.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshaderFade.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->link()) {
        close();
        return;
    }
    pNewProgram->setObjectName("Fade");
    pPrograms.append(pNewProgram); // Fade effect at 1
    pNewProgram = new QOpenGLShaderProgram(this);
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->link()) {
        close();
        return;
    }
    pNewProgram->setObjectName("Zoom Out");
    pPrograms.append(pNewProgram); // Zoom out effect at                2
    pNewProgram = new QOpenGLShaderProgram(this);
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl")) {
        close();
        return;
    }
    if (!pNewProgram->link()) {
        close();
        return;
    }
    pNewProgram->setObjectName("Zoom In");
    pPrograms.append(pNewProgram); // Zoom in  effect at                3

    nAnimationTypes = pPrograms.count();
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
    float aspectRatio = float(width())/float(height());
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
    // Transfer vertex data to VBO 0
    arrayBuf.create();
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), nVertices*sizeof(VertexData));
}


void
SlideWidget::resizeGL(int w, int h) {
    // Reset projection matrix
    projection.setToIdentity();
    float aspectRatio   = GLfloat(w)/GLfloat(h);
    float verticalAngle = 2.0*atan(1.0/viewingDistance)*180.0/M_PI;
    float nearPlane     = viewingDistance - 1.0;
    float farPlane      = viewingDistance + 1.0;
    projection.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
    if(pBaseImage) {
        delete pBaseImage;
        pBaseImage = new QImage(w,
                                h,
                                QImage::Format_RGBA8888_Premultiplied);

    }
}


void
SlideWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepthf(5.0f);

    if(currentAnimation == 0) { // Fold Effect
        pTexture0->bind(0);
        pCurrentProgram->setUniformValue(iALoc, A.x(), A.y(), A.z(), A.w());
        pCurrentProgram->setUniformValue(iThetaLoc, theta);
        pCurrentProgram->setUniformValue(iAngleLoc, angle);
        pCurrentProgram->setUniformValue(iLeftLoc, xLeft);
        matrix.setToIdentity();
        matrix.translate(0.0f, 0.0f, -viewingDistance+0.01);
        // Set modelview-projection matrix
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);

        pTexture1->bind(0);
        pCurrentProgram->setUniformValue(iALoc, A0.x(), A0.y(), A0.z(), A0.w());
        pCurrentProgram->setUniformValue(iThetaLoc, theta0);
        pCurrentProgram->setUniformValue(iAngleLoc, angle0);
        matrix.translate(0.0f, 0.0f, -0.01f);
        // Set modelview-projection matrix
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);
    } // currentAnimation == 0

    else if(currentAnimation == 1) { // Fade Effect
        pTexture0->release();
        pTexture1->release();
        pCurrentProgram->setUniformValue(iTex0Loc, 0);
        pCurrentProgram->setUniformValue(iTex1Loc, 1);
        pTexture0->bind(0);
        pTexture1->bind(1);
        iAlphaLoc = pCurrentProgram->uniformLocation("alpha");
        pCurrentProgram->setUniformValue(iAlphaLoc, alpha);
        matrix.setToIdentity();
        matrix.translate(0.0f, 0.0f, -viewingDistance);
        matrix.rotate(rotation);
        // Set modelview-projection matrix
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);
    } // currentAnimation == 1

    else if(currentAnimation == 2) { // Zoom out effect
        pCurrentProgram->setUniformValue(iTex0Loc, 0);
        pTexture0->bind(0);
        matrix.setToIdentity();
        matrix.translate(0.0f, 0.0f, -viewingDistance+0.01);
        matrix.scale(fScale);
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);

        pCurrentProgram->setUniformValue(iTex1Loc, 0);
        pTexture1->bind(0);
        matrix.setToIdentity();
        matrix.translate(0.0f, 0.0f, -viewingDistance);
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);
    } // currentAnimation == 2

    else if(currentAnimation == 3) { // Zoom in  effect
        pCurrentProgram->setUniformValue(iTex0Loc, 0);
        pTexture0->bind(0);
        matrix.setToIdentity();
        matrix.translate(0.0f, 0.0f, -viewingDistance-0.01);
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);

        pCurrentProgram->setUniformValue(iTex1Loc, 0);
        pTexture1->bind(0);
        matrix.translate(0.0f, 0.0f, 0.01f);
        matrix.scale(1.0f-fScale);
        // Set modelview-projection matrix
        pCurrentProgram->setUniformValue("mvp_matrix", projection * matrix);
        drawGeometry(pCurrentProgram);
    } // currentAnimation == 3

    glDisable(GL_DEPTH_TEST);
}


void
SlideWidget::ontimerAnimateEvent() {
    if(currentAnimation == 0) {
        A += QVector4D(0.0f, -0.02f, 0.0f, 0.0f);
        if(theta > 0.2f)
            theta -= 0.04f;
        else if(angle < M_PI_2)
            angle+= 0.15f;
        if(A.y() < -1.88f) {
            timerAnimate.stop();
            prepareNextRound();
            A      = A0;
            theta  = theta0;
            angle  = angle0;
        }
    }
    else if(currentAnimation == 1) {
        alpha -= 0.02f;
        if(alpha < 0.0) {
            timerAnimate.stop();
            prepareNextRound();
            alpha = alpha0;
        }
    }
    else if(currentAnimation == 2) {
        fScale -= 0.02f;
        if(fScale <= 0.0) {
            timerAnimate.stop();
            prepareNextRound();
            fScale = fScale0;
        }
    }
    else if(currentAnimation == 3) {
            fScale -= 0.02f;
            if(fScale <= 0.0) {
                timerAnimate.stop();
                prepareNextRound();
                fScale = fScale0;
            }
    }
    else if(currentAnimation == 4) {
            fRot += 2.0f;
            if(fRot > 90.0) {
                timerAnimate.stop();
                prepareNextRound();
                fRot = fRot0;
            }
    }
    else if(currentAnimation == 5) {
            fRot += 2.0f;
            if(fRot > 90.0) {
                timerAnimate.stop();
                prepareNextRound();
                fRot = fRot0;
            }
    }
    update();
}


void
SlideWidget::onTimerSteadyEvent() {
    timerAnimate.start(UPDATE_TIME);
}

