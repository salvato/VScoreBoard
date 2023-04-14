#pragma once


class PixelData
{
public:
    PixelData();
    ~PixelData();

public:
    enum {
        eWrongIndex = 0xFFFFFFFF
    };
    enum ESide {
        eLeft,
        eTop,
        eRight,
        eBottom,

        eSideCount
    };
    enum ECorner {
        eTopLeft,
        eTopRight,
        eBottomRight,
        eBottomLeft,

        eCornerCount
    };

public:
    unsigned int    m_x;
    unsigned int    m_y;
    bool            m_edge[eSideCount];
    unsigned int    m_pointIndex[eCornerCount];
    bool            m_corner[eCornerCount];
};
