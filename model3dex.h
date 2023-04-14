#pragma once

#include "model3d.h"


class Model3DEx : public Model3D
{
public:
    Model3DEx();
    ~Model3DEx();

public:
    void Recalc();
    bool GetRadius(float& radius) const;
    bool GetCenter(float& x, float& y, float& z) const;
    bool GetMin(float& x, float& y, float& z) const;
    bool GetMax(float& x, float& y, float& z) const;

private:
    float   m_center[3];
    float   m_minCoord[3];
    float   m_maxCoord[3];
    float   m_radius;
};
