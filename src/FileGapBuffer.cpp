#include "FileGapBuffer.h"
#include <ncurses.h>

#include <utility>

FileGapBuffer::FileGapBuffer(int initialSize)
    : m_buffer(std::vector<std::shared_ptr<LineGapBuffer>>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
}

void FileGapBuffer::up()
{
    if (m_preGapIndex != 0)
    {
        m_preGapIndex--;
        m_postGapIndex--;
        m_buffer[m_postGapIndex] = std::move(m_buffer[m_preGapIndex]);
    }
}

void FileGapBuffer::down()
{
    if (m_postGapIndex < m_bufferSize)
    {
        m_buffer[m_preGapIndex] = std::move(m_buffer[m_postGapIndex]);
        m_preGapIndex++;
        m_postGapIndex++;
    }
}

void FileGapBuffer::insertLine(const std::shared_ptr<LineGapBuffer>& line)
{
    if (m_preGapIndex == m_postGapIndex)
    {
        grow();
    }
    else if (m_preGapIndex > m_postGapIndex)
    {
        std::cerr << "Pre gap index bugger than post gap index!\n";

        abort();
    }

    m_buffer[m_preGapIndex] = std::move(line);
    m_preGapIndex++;
}

std::shared_ptr<LineGapBuffer> FileGapBuffer::deleteLine()
{
    if (m_preGapIndex > 0)
    {
        return m_buffer[--m_preGapIndex];
    }
    else
    {
        return std::shared_ptr<LineGapBuffer>();
    }
}

void FileGapBuffer::grow()
{
    int newSize = m_bufferSize * 2;

    std::vector<std::shared_ptr<LineGapBuffer>> newBuffer(newSize);

    std::move(m_buffer.begin(), m_buffer.begin() + m_preGapIndex, newBuffer.begin());
    std::move(m_buffer.begin() + m_preGapIndex, m_buffer.end(), newBuffer.begin() + newSize / 2 + m_preGapIndex);

    m_postGapIndex = newSize / 2 + m_preGapIndex;

    m_buffer = std::move(newBuffer);
    m_bufferSize = newSize;
}

const std::shared_ptr<LineGapBuffer>& FileGapBuffer::operator [](size_t index) const
{
    // assert(index < m_bufferSize - m_postGapIndex + m_preGapIndex);
    try
    {
        if (!(index < m_bufferSize - m_postGapIndex + m_preGapIndex))
        {
            throw (index);
        }
    }
    catch (size_t)
    {
        endwin();
        std::cerr << "Index out of bounds: " << index << '\n';
        std::cout << "File Gap Buffer Info:\n\t" << "File Size: " << m_bufferSize << "\n\t";
        std::cout << "Num lines: " << numberOfLines() << "\n\t";
        std::cout << "Pre index: " << preGapIndex() << "\n\t";
        std::cout << "Post index: " << postGapIndex() << "\n";

        abort();
    }

    if (index < m_preGapIndex)
    {
        return m_buffer[index];
    }
    else
    {
        return m_buffer[index + m_postGapIndex - m_preGapIndex];
    }
}
