#include "Clipboard.h"

void Clipboard::add(const std::shared_ptr<LineGapBuffer>& line)
{
    if (m_fullLines.size() == m_numberOfLines)
    {
        m_fullLines.push_back(*line);
    }
    else
    {
        m_fullLines[m_numberOfLines] = *line;
    }

    m_numberOfLines++;
}

void Clipboard::lineUpdate()
{
    m_numberOfLines = 0;
    m_yankType = LINE_YANK;
}

void Clipboard::visualUpdate(const int initialX, const int finalX, const int boundDifferenceY)
{
    m_numberOfLines = 0;
    m_yankType = VISUAL_YANK;
    m_initialX = initialX;
    m_finalX = finalX;
    m_differenceBoundsY = boundDifferenceY;
}

void Clipboard::blockUpdate(const int initialX, const int finalX)
{
    m_numberOfLines = 0;
    m_yankType = BLOCK_YANK;
    m_initialX = initialX;
    m_finalX = finalX;
}

const LineGapBuffer& Clipboard::operator [] (size_t index) const
{
    if (index < m_numberOfLines)
    {
        return m_fullLines[index];
    }
    else
    {
        endwin();
        std::cerr << "Incorrect clipboard getter-index: " << index << '\n';
        exit(1);
    }
}
