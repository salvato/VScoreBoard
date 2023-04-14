
#include "model3dex.h"
#include "qmath.h"


Model3DEx::Model3DEx() {
    Recalc();
}


Model3DEx::~Model3DEx() {
}


void
Model3DEx::Recalc() {
    if(!IsValid()) {
        m_radius = 0.0f;
        for(int coord=0; coord<3; ++coord) {
            m_center[coord]   = 0.0f;
            m_minCoord[coord] = 0.0f;
            m_maxCoord[coord] = 0.0f;
        }
        return;
    }
    unsigned int pointCount = GetPointCount();
    const float* pointCoord = GetPoints();
    for(int coord=0; coord<3 ; ++coord) {
        m_minCoord[coord] = pointCoord[coord];
        m_maxCoord[coord] = pointCoord[coord];
    }
    pointCoord += 3;
    pointCount -= 1;
    for(; pointCount > 0 ; --pointCount, pointCoord += 3) {
        for(int coord=0; coord<3; ++coord) {
            m_minCoord[coord] = qMin(pointCoord[coord], m_minCoord[coord]);
            m_maxCoord[coord] = qMax(pointCoord[coord], m_maxCoord[coord]);
        }
    }
    double radiusSq = 0.0;
    for(int coord=0; coord<3 ; ++coord) {
        m_center[coord] = ((m_minCoord[coord] + m_maxCoord[coord]) / 2.0);
        double radiusCoord = ((m_maxCoord[coord] - m_minCoord[coord]) / 2.0);
        radiusSq += (radiusCoord * radiusCoord);
    }
    m_radius = static_cast<float>(qSqrt(radiusSq));
}


bool
Model3DEx::GetRadius(float& radius) const
{
    if(!IsValid()) {
        return false;
    }
    radius = m_radius;
    return true;
}


bool
Model3DEx::GetCenter(float& x, float& y, float& z) const
{
    if(!IsValid()) {
        return false;
    }
    x = m_center[0];
    y = m_center[1];
    z = m_center[2];
    return true;
}


bool
Model3DEx::GetMin(float& x, float& y, float& z) const
{
    if(!IsValid()) {
        return false;
    }
    x = m_minCoord[0];
    y = m_minCoord[1];
    z = m_minCoord[2];
    return true;
}


bool
Model3DEx::GetMax(float& x, float& y, float& z) const
{
    if(!IsValid()) {
        return false;
    }
    x = m_maxCoord[0];
    y = m_maxCoord[1];
    z = m_maxCoord[2];
    return true;
}
