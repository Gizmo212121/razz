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
