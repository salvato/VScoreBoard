////////
// This sample is published as part of the blog article at www.toptal.com/blog
// Visit www.toptal.com/blog and subscribe to our newsletter to read great posts
////////

#pragma once

#include <QString>

struct Model3DData
{
    unsigned int    m_triangleCount;
    unsigned int    m_pointCount;
    unsigned int*   m_pointIndices;
    float*          m_pointCoords;
};

class Model3D : private Model3DData
{
public:
    Model3D();
    virtual ~Model3D();

public:
    bool IsValid() const;
    void Release();
    Model3D& operator=(const Model3D& modelSrc);
    Model3D& Swap(Model3D& modelSrc);
    void Assign(Model3DData& modelData);
    bool Load(const QString& sFilePath);
    bool Save(const QString& sFilePath) const;

public:
    unsigned int GetTriangleCount() const   {   return m_triangleCount; };
    unsigned int GetPointCount() const      {   return m_pointCount;    };
    const unsigned int* GetIndexes() const  {   return m_pointIndices;  };
    const float* GetPoints() const          {   return m_pointCoords;   };
};

