////////
// This sample is published as part of the blog article at www.toptal.com/blog
// Visit www.toptal.com/blog and subscribe to our newsletter to read great posts
////////

#pragma once

#include "model3d.h"
#include "pixeldata.h"


#include <QImage>
#include <QByteArray>
#include <QBitArray>

class ModelCreator
{
public:
    ModelCreator(const QImage &image, float depth=0.2f);
    ~ModelCreator();

public:
    Model3D& Create3DModel();

private:
    void GetPixelCount();
    void CreatePixelData();
    void CreateBinaryData();

private:
    const QImage&   m_image;
    Model3D         m_model;
    QBitArray       m_pixelSet;
    uint            m_pixelCount;
    PixelData*      m_pixelData;
    uint            m_pointCount;
    QBitArray       m_cornerSet;
    uint            m_cornerCount;
    uint            m_edgeCount;
    float           m_depth;
};
