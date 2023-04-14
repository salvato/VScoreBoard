////////
// This sample is published as part of the blog article at www.toptal.com/blog
// Visit www.toptal.com/blog and subscribe to our newsletter to read great posts
////////

#include "model3d.h"

#include <windows.h>
#include <QtGlobal>
#include <QtMath>
#include <QFile>


Model3D::Model3D() {
    m_triangleCount = 0;
    m_pointCount = 0;
    m_pointIndices = 0;
    m_pointCoords = 0;
}


Model3D::~Model3D() {
    Release();
}


bool
Model3D::IsValid() const
{
    return ((0 != m_triangleCount) &&
            (0 != m_pointCount) &&
            (0 != m_pointIndices) &&
            (0 != m_pointCoords));
}


void
Model3D::Release() {
    m_triangleCount = 0;
    m_pointCount = 0;
    delete [] m_pointIndices;
    m_pointIndices = 0;
    delete [] m_pointCoords;
    m_pointCoords = 0;
}


Model3D&
Model3D::operator=(const Model3D& modelSrc) {
    Release();
    m_pointIndices = new unsigned int[3 * m_triangleCount];
    m_pointCoords = new float[3 * m_pointCount];
    if((0 == m_pointIndices) || (0 == m_pointCoords)) {
        Release();
        return (*this);
    }
    memcpy(m_pointIndices, modelSrc.m_pointIndices,
           (sizeof(unsigned int) * 3 * m_triangleCount));
    memcpy(m_pointCoords, modelSrc.m_pointCoords,
           (sizeof(float) * 3 * m_pointCount));
    return (*this);
}


Model3D&
Model3D::Swap(Model3D& modelSrc) {
    uint triangleCount = m_triangleCount;
    uint pointCount    = m_pointCount;
    uint* pointIndices = m_pointIndices;
    float* pointCoords = m_pointCoords;
    m_triangleCount = modelSrc.m_triangleCount;
    m_pointCount    = modelSrc.m_pointCount;
    m_pointIndices  = modelSrc.m_pointIndices;
    m_pointCoords   = modelSrc.m_pointCoords;
    modelSrc.m_triangleCount = triangleCount;
    modelSrc.m_pointCount    = pointCount;
    modelSrc.m_pointIndices  = pointIndices;
    modelSrc.m_pointCoords   = pointCoords;
    return (*this);
}


void
Model3D::Assign(Model3DData& modelData) {
    Release();
    (*dynamic_cast<Model3DData*>(this)) = modelData;
    modelData.m_triangleCount = 0;
    modelData.m_pointCount    = 0;
    modelData.m_pointIndices  = 0;
    modelData.m_pointCoords   = 0;
}


bool
Model3D::Load(const QString& sFilePath) {
    Release();
    QFile file(sFilePath);
    if(file.open(QIODevice::ReadOnly)) {
        if(file.size() > 2 * sizeof(unsigned int)) {
            file.read(
                static_cast<char*>(static_cast<void*>(&m_triangleCount)),
                    sizeof(unsigned int));
            file.read(
                static_cast<char*>(static_cast<void*>(&m_pointCount)),
                    sizeof(unsigned int));
            if(file.size() == ((2 * sizeof(unsigned int)) +
                sizeof(unsigned int) * m_triangleCount * 3 +
                sizeof(float) * m_pointCount * 3))
            {
                m_pointIndices = new unsigned int[m_triangleCount * 3];
                file.read(
                    static_cast<char*>(static_cast<void*>(m_pointIndices)),
                        sizeof(unsigned int) * m_triangleCount * 3);
                m_pointCoords = new float[m_pointCount * 3];
                file.read(
                    static_cast<char*>(static_cast<void*>(m_pointCoords)),
                        sizeof(float) * m_pointCount * 3);

                file.close();
                return true;
            }
        }
        file.close();
    }
    return false;
}


bool
Model3D::Save(const QString& sFilePath) const
{
    QFile file(sFilePath);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(
            static_cast<const char*>(static_cast<const void*>(&m_triangleCount)),
                sizeof(unsigned int));
        file.write(
            static_cast<const char*>(static_cast<const void*>(&m_pointCount)),
                sizeof(unsigned int));
        file.write(
            static_cast<const char*>(static_cast<const void*>(m_pointIndices)),
                sizeof(unsigned int) * m_triangleCount * 3);
        file.write(
            static_cast<const char*>(static_cast<const void*>(m_pointCoords)),
                sizeof(float) * m_pointCount * 3);

        file.close();
        return true;
    }
    return false;
}

