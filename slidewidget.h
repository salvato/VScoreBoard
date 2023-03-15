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
    bool setSlideDir(QString sNewDir);
    bool startSlideShow();
    void stopSlideShow();

protected:
    void initializeGL() override;
    void paintGL() override;

    void initShaders();
    void initTextures();
    bool getLocations();

    bool prepareNextRound() ;
    bool prepareNextSlide();

public slots:
    void ontimerAnimateEvent();
    void onTimerSteadyEvent();
    void closeEvent(QCloseEvent*) override;

private:
    void initGeometry();
    void drawGeometry(QOpenGLShaderProgram *program);
    bool updateSlideList();

private:
    struct VertexData {
        QVector3D position;
        QVector2D texCoord;
    };
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
    GLint     iTex0Loc;
    GLint     iTex1Loc;
    float viewingDistance;
    QMatrix4x4 matrix;
    GLint     iProgressLoc;
    GLfloat   progress;
    GLfloat   aspectRatio;
};
