////////
// This sample is published as part of the blog article at www.toptal.com/blog
// Visit www.toptal.com/blog and subscribe to our newsletter to read great posts
////////

#include "model_creator.h"

#include <QVector>
#include <QDebug>



ModelCreator::ModelCreator(const QImage& image, float depth)
    : m_image(image)
    , m_pixelCount(0)
    , m_pixelData(0)
    , m_pointCount(0)
    , m_cornerCount(0)
    , m_edgeCount(0)
    , m_depth(depth)
{
}


ModelCreator::~ModelCreator() {
    delete[] m_pixelData;
}


Model3D&
ModelCreator::Create3DModel() {
    GetPixelCount();
    CreatePixelData();
    CreateBinaryData();
    return m_model;
}


// Crea un QBitArray(m_pixelSet) con 0 se il pixel corrispondente
// dell'immagine fa parte del background e 1 se fa parte del carattere
// e conta il numero di pixel (m_pixelCount) che costituiscono il carattere.
void
ModelCreator::GetPixelCount() {
    m_pixelSet.fill(false, m_image.height() * m_image.width());
    m_pixelCount = 0;
    uint pixel = 0;
    for(int y=0; y<m_image.height(); ++y) {
        const uchar* dataPtr = m_image.constScanLine(y);
        uchar data = 0;
        for(int x=0; x<m_image.width(); ++x, ++pixel) {
            if(0 == (x % (sizeof(uchar) * 8))) {
                data = (*dataPtr);
                ++dataPtr;
            }
            else {
                data = (data << 1);
            }
            if(0 == (data & 0x80)) {
                m_pixelSet.setBit(pixel);
                ++m_pixelCount;
            }
        }
    }
}


void
ModelCreator::CreatePixelData() {
    delete [] m_pixelData;
    m_pixelData = new PixelData[m_pixelCount];

    const int h = m_image.height();
    const int w = m_image.width();

    uint pixel = 0;
    uint pixelIndex = 0;
    QVector<uint> pointIndexesTop((1 + w), PixelData::eWrongIndex);
    m_pointCount = 0;
    for(int y=0; y<h; ++y) {
        QVector<uint> pointIndexesBottom((1 + w), PixelData::eWrongIndex);
        for(int x=0; x<w; ++x, ++pixel) {
            if(m_pixelSet.testBit(pixel)) {
                PixelData& pixelData = m_pixelData[pixelIndex++];
                pixelData.m_edge[PixelData::eLeft] =
                    ((0 == x) ? true : !m_pixelSet.testBit(w*y + x-1));
                pixelData.m_edge[PixelData::eTop] =
                    ((0 == y) ? true : !m_pixelSet.testBit(w*(y-1) + x));
                pixelData.m_edge[PixelData::eRight] =
                    ((w == (1+x)) ? true : !m_pixelSet.testBit(w*y + x+1));
                pixelData.m_edge[PixelData::eBottom] =
                    ((h == (1+y)) ? true : !m_pixelSet.testBit(w*(y+1) + x));

                pixelData.m_x = x;
                pixelData.m_y = y;

                pixelData.m_corner[PixelData::eTopLeft] =
                    (pixelData.m_edge[PixelData::eLeft] ||
                     pixelData.m_edge[PixelData::eTop]);
                pixelData.m_corner[PixelData::eTopRight] =
                    (pixelData.m_edge[PixelData::eRight] ||
                     pixelData.m_edge[PixelData::eTop]);
                pixelData.m_corner[PixelData::eBottomRight] =
                    (pixelData.m_edge[PixelData::eRight] ||
                     pixelData.m_edge[PixelData::eBottom]);
                pixelData.m_corner[PixelData::eBottomLeft] =
                    (pixelData.m_edge[PixelData::eLeft] ||
                     pixelData.m_edge[PixelData::eBottom]);

                pixelData.m_pointIndex[PixelData::eTopLeft] =
                    ((PixelData::eWrongIndex != pointIndexesTop[x])
                    ? pointIndexesTop[x]
                    : (m_pointCount++));

                pixelData.m_pointIndex[PixelData::eTopRight] =
                    ((PixelData::eWrongIndex != pointIndexesTop[1 + x])
                    ? pointIndexesTop[1 + x]
                    : (m_pointCount++));

                pointIndexesTop[1+x] = pixelData.m_pointIndex[PixelData::eTopRight];

                pixelData.m_pointIndex[PixelData::eBottomLeft] =
                    ((PixelData::eWrongIndex != pointIndexesBottom[x])
                    ? pointIndexesBottom[x]
                    : (m_pointCount++));

                pointIndexesBottom[x] = pixelData.m_pointIndex[PixelData::eBottomLeft];

                pixelData.m_pointIndex[PixelData::eBottomRight] =
                    (m_pointCount++);

                pointIndexesBottom[1 + x] = pixelData.m_pointIndex[PixelData::eBottomRight];
            }
        }
        pointIndexesTop = pointIndexesBottom;
    }

    m_cornerSet.fill(false, m_pointCount);
    m_edgeCount = 0;
    for(uint pixelIndex=0; pixelIndex<m_pixelCount ; ++pixelIndex) {
        const PixelData& pixelData = m_pixelData[pixelIndex];
        for(uint corner=0; corner<PixelData::eCornerCount; ++corner ) {
            if(pixelData.m_corner[corner]) {
                m_cornerSet.setBit(pixelData.m_pointIndex[corner]);
            }
        }
        for(uint side=0; side<PixelData::eSideCount; ++side) {
            if(pixelData.m_edge[side]) {
                ++m_edgeCount;
            }
        }
    }

    m_cornerCount = m_cornerSet.count(true);
}


void
ModelCreator::CreateBinaryData() {
    Model3DData modelData =
    {
        (2 * ((2*m_pixelCount) + m_edgeCount)),
        (2 * m_pointCount),
        0,
        new float[2*3* m_pointCount]
    };
    modelData.m_pointIndices = new uint[3 * modelData.m_triangleCount];

    double stepOne = (1.0 / static_cast<double>(m_image.height()));

    uint* pointIndices = modelData.m_pointIndices;
    for(uint pixelIndex=0; pixelIndex<m_pixelCount; ++pixelIndex) {
        const PixelData& pixelData = m_pixelData[pixelIndex];

        (*pointIndices) = pixelData.m_pointIndex[PixelData::eBottomLeft];
        pointIndices++;
        (*pointIndices) = pixelData.m_pointIndex[PixelData::eTopLeft];
        pointIndices++;
        (*pointIndices) = pixelData.m_pointIndex[PixelData::eTopRight];
        pointIndices++;

        (*pointIndices) = pixelData.m_pointIndex[PixelData::eTopRight];
        pointIndices++;
        (*pointIndices) = pixelData.m_pointIndex[PixelData::eBottomRight];
        pointIndices++;
        (*pointIndices) = pixelData.m_pointIndex[PixelData::eBottomLeft];
        pointIndices++;

        for(uint corner=0; corner<PixelData::eCornerCount; ++corner) {
            bool right    = ((PixelData::eTopRight == corner) || (PixelData::eBottomRight == corner));
            bool bottom   = ((PixelData::eBottomLeft == corner) || (PixelData::eBottomRight == corner));
            float* buffer = modelData.m_pointCoords + (3 * pixelData.m_pointIndex[corner]);
            buffer[0] = static_cast<float>(stepOne * (pixelData.m_x + (right ? 1 : 0)));
            buffer[1] = static_cast<float>(stepOne * (m_image.height() - (pixelData.m_y + (bottom ? 1 : 0))));
            buffer[2] = 0.0f;
        }
    }

    Q_ASSERT(pointIndices == modelData.m_pointIndices + 6 * m_pixelCount);

    uint* pointIndicesSrc = modelData.m_pointIndices;
    uint* pointIndicesDest = (modelData.m_pointIndices + 6 * m_pixelCount);
    for(uint pointIndex=0; pointIndex<(m_pixelCount * 6); ++pointIndex) {
        (*(pointIndicesDest++)) = (*(pointIndicesSrc++)) + m_pointCount;
    }

    const float* pointCoordSrc = modelData.m_pointCoords;
    float* pointCoordDest = (modelData.m_pointCoords + 3*m_pointCount);
    for(uint point=0; point<m_pointCount; ++point) {
        (*(pointCoordDest++)) = (*(pointCoordSrc++));
        (*(pointCoordDest++)) = (*(pointCoordSrc++));
        (*(pointCoordDest++)) = m_depth;
        ++pointCoordSrc;
    }

    uint edgeIndex = 0;
    uint* point = (modelData.m_pointIndices + (3 * 2 * 2 * m_pixelCount));
    for(uint pixelIndex=0; pixelIndex<m_pixelCount; ++pixelIndex) {
        const PixelData& pixelData = m_pixelData[pixelIndex];
        for(uint side=0; side<PixelData::eSideCount; ++side) {
            if(pixelData.m_edge[side]) {
                PixelData::ECorner corners[2] =
                {
                    static_cast<PixelData::ECorner>(side),
                    ((side == PixelData::eLeft)
                        ? PixelData::eBottomLeft
                        : static_cast<PixelData::ECorner>(side - 1))
                };
                uint pointIndexes[4] =
                {
                    pixelData.m_pointIndex[corners[0]],
                    pixelData.m_pointIndex[corners[1]],
                    pixelData.m_pointIndex[corners[1]] + m_pointCount,
                    pixelData.m_pointIndex[corners[0]] + m_pointCount
                };
                (*(point++)) = pointIndexes[0];
                (*(point++)) = pointIndexes[1];
                (*(point++)) = pointIndexes[2];
                (*(point++)) = pointIndexes[2];
                (*(point++)) = pointIndexes[3];
                (*(point++)) = pointIndexes[0];

                ++edgeIndex;
            }
        }
    }
    Q_ASSERT(edgeIndex == m_edgeCount);
    m_model.Assign(modelData);
}
