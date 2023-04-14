#include "pixeldata.h"
#include "qglobal.h"

PixelData::PixelData()
    :   m_x(0)
    ,   m_y(0)
{
    for(uint side=0; side<eSideCount; ++side) {
        m_edge[side] = false;
    }
    for(uint corner=0; corner<eCornerCount ; ++corner) {
        m_pointIndex[corner] = eWrongIndex;
        m_corner[corner] = true;
    }
}


PixelData::~PixelData() {
}

