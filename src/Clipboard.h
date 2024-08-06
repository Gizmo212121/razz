#pragma once

#include "Includes.h"
#include "LineGapBuffer.h"

class Clipboard
{

private:

    YANK_TYPE m_yankType = YANK_TYPE::LINE_YANK;

    size_t m_numberOfLines = 0;
    std::vector<LineGapBuffer> m_fullLines;

    int m_initialX = 0;
    int m_finalX = 0;
    int m_initialY = 0;
    int m_finalY = 0;
    int m_differenceBoundsY = 0;

public:

    void add(const std::shared_ptr<LineGapBuffer>& line);

    void lineUpdate();
    // Lower and upper bounds should be inclusive
    void visualUpdate(const int initialX, const int finalX, const int initialY, const int finalY);
    void blockUpdate(const int initialX, const int finalX, const int initialY, const int finalY);

    // Getters
    size_t numberOfLines() const { return m_numberOfLines; }
    const LineGapBuffer& operator [] (size_t index) const;
    YANK_TYPE yankType() const { return m_yankType; }
    void copy(std::vector<LineGapBuffer>& vec) const;

    int initialX() const { return m_initialX; }
    int finalX() const { return m_finalX; }

    int initialY() const { return m_initialY; }
    int finalY() const { return m_finalY; }

};
