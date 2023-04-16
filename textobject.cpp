#include "textobject.h"
#include "model_creator.h"

#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QOpenGLShaderProgram>
#include <QOpenglTexture>


TextObject::TextObject(QSizeF                _size,
                       QOpenGLShaderProgram* _pProgram,
                       QOpenGLTexture*       _pTexture,
                       QVector3D             _position,
                       QQuaternion           _rotation,
                       QVector3D             _scale,
                       QVector3D             _speed)
    : Object(_pProgram,
             _pTexture,
             _position,
             _rotation,
             _scale,
             _speed)
    , sText(QString())
    , height(96)
    , depth(0.1f)
    , size(_size)
{
    initializeOpenGLFunctions();
    QString sFontName = QString("Arial");
    font = QFont(sFontName, height, QFont::Light);
}


void
TextObject::setText(QString _sText,
                    QColor  _color,
                    ushort  _height,
                    float   _depth) {
    sText  = _sText;
    color  = _color;
    height = _height;
    depth  = _depth;
    create2DImage(sText, font, height);
    QPixmap pixmap;
    pixmap.convertFromImage(*pTextImage, Qt::MonoOnly);
    pixmap.setMask(pixmap.createMaskFromColor(QColor(Qt::white), Qt::MaskInColor));
    ModelCreator creator(*pTextImage, depth);
    textModel.Swap(creator.Create3DModel());
    createTextModel();
}


void
TextObject::draw() {
    draw(pProgram);
}


void
TextObject::draw(QOpenGLShaderProgram* pOtherProgram) {
    if(textModel.GetTriangleCount()) {
        glDisable(GL_CULL_FACE);
        pOtherProgram->bind();
        pOtherProgram->setUniformValue("fragmentColor",
                                       static_cast<GLfloat>(color.red())   / 256.0f,
                                       static_cast<GLfloat>(color.green()) / 256.0f,
                                       static_cast<GLfloat>(color.blue())  / 256.0f);
        pOtherProgram->setUniformValue("model", modelMatrix());

        vertexTextBuf.bind();
        pOtherProgram->enableAttributeArray("vPosition");
        pOtherProgram->setAttributeBuffer("vPosition", GL_FLOAT, 0, 3, 3*sizeof(float));

        normalTextBuf.bind();
        pOtherProgram->enableAttributeArray("vNormal");
        pOtherProgram->setAttributeBuffer("vNormal", GL_FLOAT, 0, 3, 3*sizeof(float));

        glDrawArrays(GL_TRIANGLES, 0, 3*textModel.GetTriangleCount());
        glEnable(GL_CULL_FACE);
    }
}


float*
TextObject::generateVertexBuffer(const Model3D& model) {
    const unsigned int triangleCount = model.GetTriangleCount();
    float* pointCoord = new float[3 * 3 * triangleCount];

    if(pointCoord != nullptr) {
        const unsigned int* indexes = model.GetIndexes();
        const float* pointCoordSrc = model.GetPoints();
        float* coord = pointCoord;
        for(uint triangle=0; triangle<triangleCount; ++triangle) {
            for(uint vertex=0; vertex<3; ++vertex, ++indexes) {
                const float* coordSrc = (pointCoordSrc + (3 * (*indexes)));
                (*(coord++)) = (*(coordSrc++));
                (*(coord++)) = (*(coordSrc++));
                (*(coord++)) = (*(coordSrc++));
            }
        }
    }
    return pointCoord;
}


void
TextObject::generateNormalsBuffer(const Model3D& model, float* coords) {
    const unsigned int triangleCount = model.GetTriangleCount();
    const unsigned int* indexes = model.GetIndexes();
    const float* points = model.GetPoints();
    for(uint triangle=0; triangle<triangleCount; ++triangle, indexes += 3) {
        const float* pointCoord1 = (points + (3 * indexes[0]));
        const float* pointCoord2 = (points + (3 * indexes[1]));
        const float* pointCoord3 = (points + (3 * indexes[2]));
        QVector3D
            normal(QVector3D::crossProduct(QVector3D((pointCoord2[0] - pointCoord1[0]),
                                                     (pointCoord2[1] - pointCoord1[1]),
                                                     (pointCoord2[2] - pointCoord1[2])),
                                           QVector3D((pointCoord3[0] - pointCoord1[0]),
                                                     (pointCoord3[1] - pointCoord1[1]),
                                                     (pointCoord3[2] - pointCoord1[2]))
                                           ).normalized());
        (*(coords++)) = normal[0];
        (*(coords++)) = normal[1];
        (*(coords++)) = normal[2];
        (*(coords++)) = normal[0];
        (*(coords++)) = normal[1];
        (*(coords++)) = normal[2];
        (*(coords++)) = normal[0];
        (*(coords++)) = normal[1];
        (*(coords++)) = normal[2];
    }
}


void
TextObject::create2DImage(const QString& sText, const QFont& font, ushort height)
{
    qreal coeff = (static_cast<qreal>(QFontMetrics(font).height()) / static_cast<qreal>(height));
    qreal newFontPointSize = (font.pointSizeF() / coeff);

    QFont fontScaled(font);
    fontScaled.setPointSizeF(newFontPointSize);

    int width = (3 + QFontMetrics(fontScaled).boundingRect(sText).width());

    if(pTextImage) delete pTextImage;
    pTextImage = new QImage(width, height, QImage::Format_Mono);
    pTextImage->fill((uint)1);
    {
        QPainter painter(pTextImage);
        painter.setFont(fontScaled);
        painter.setPen(QColor(Qt::black));

        const QRect rectDraw(QPoint(0, 0), QSize(width, height));
        painter.drawText(rectDraw, (Qt::AlignLeft | Qt::AlignTop), sText);
    }
}


void
TextObject::createTextModel() {
    textModel.Recalc();
    float* coords = generateVertexBuffer(textModel);
    if(coords != nullptr) {
        vertexTextBuf.destroy();
        normalTextBuf.destroy();
        vertexTextBuf.create();
        normalTextBuf.create();
        if(vertexTextBuf.isCreated() && normalTextBuf.isCreated()) {
            vertexTextBuf.bind();
            vertexTextBuf.allocate(coords, 3*3*textModel.GetTriangleCount()*sizeof(float));
            vertexTextBuf.release();

            generateNormalsBuffer(textModel, coords);
            normalTextBuf.bind();
            normalTextBuf.allocate(coords, 3*3*textModel.GetTriangleCount()*sizeof(float));
            normalTextBuf.release();
        }
        else {
            vertexTextBuf.destroy();
            normalTextBuf.destroy();
            vertexTextBuf.create();
            normalTextBuf.create();
        }
        delete[] coords;
    }
}


void
TextObject::updateStatus(float deltaTime) {
    position += speed*deltaTime;
}
