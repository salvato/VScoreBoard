#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QFileInfoList>
#include <QTimer>


class SlideWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    SlideWidget();
    ~SlideWidget();

public:
    void setSlideDir(QString sNewDir);
    void startSlideShow();
    void stopSlideShow();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();
    void initTextures();
    bool getLocations();

    bool prepareNextRound() ;
    bool prepareNextSlide();

public slots:
    void ontimerAnimateEvent();
    void onTimerSteadyEvent();

private:
    void initGeometry();
    void drawGeometry(QOpenGLShaderProgram *program);
    void updateSlideList();

private:
    struct VertexData
    {
        QVector3D position;
        QVector2D texCoord;
    };
    QBasicTimer timer;
    QTimer timerAnimate;
    QTimer timerSteady;
    QVector<QOpenGLShaderProgram*> pPrograms;
    QOpenGLShaderProgram* pCurrentProgram;

    QOpenGLBuffer arrayBuf;
    QOpenGLTexture* pTexture0 = nullptr;
    QOpenGLTexture* pTexture1 = nullptr;

    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed = 0;
    QQuaternion rotation;

    QString sSlideDir;
    int iCurrentSlide;
    QFileInfoList slideList;
    int nVertices;
    int nAnimationTypes;
    bool bRunning;
    QImage* pBaseImage;
    QImage image;
    int currentAnimation;
    QVector4D A;
    QVector4D A0;
    GLfloat   theta;
    GLfloat   theta0;
    GLfloat   angle;
    GLfloat   angle0;
    GLfloat   alpha;
    GLfloat   alpha0;
    GLfloat   fScale;
    GLfloat   fScale0;
    GLfloat   fRot;
    GLfloat   fRot0;
    GLint     iLeftLoc;
    GLint     iAlphaLoc;
    GLint     iALoc;
    GLint     iThetaLoc;
    GLint     iAngleLoc;
    GLint     iTex0Loc;
    GLint     iTex1Loc;
    GLfloat   xLeft;
    float viewingDistance;
    QMatrix4x4 matrix;
};
