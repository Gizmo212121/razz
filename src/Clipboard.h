#pragma once

#include "Includes.h"
#include "LineGapBuffer.h"

class Clipboard
{

private:

    size_t m_numberOfLines = 0;
    std::vector<LineGapBuffer> m_fullLines;

public:

    void add(const std::shared_ptr<LineGapBuffer>& line);

    void clear() { m_numberOfLines = 0; }

    // Getters
    size_t numberOfLines() const { return m_numberOfLines; }
    const LineGapBuffer& operator [] (size_t index) const { return m_fullLines[index]; }

};
