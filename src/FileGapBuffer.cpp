#include "FileGapBuffer.h"
#include <cstdlib>
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

void FileGapBuffer::swapLinesInRange(bool right, int start, int end)
{
    if (static_cast<size_t>(right) && static_cast<size_t>(end) == m_bufferSize - m_postGapIndex + m_preGapIndex - 1) { return; }
    if (!right && start == 0) { return; }
    if (end - start < 0) { return; }

    size_t absoluteStart = (static_cast<size_t>(start) < m_preGapIndex) ? static_cast<size_t>(start) : static_cast<size_t>(start) + m_postGapIndex - m_preGapIndex;
    size_t absoluteEnd = (static_cast<size_t>(end) < m_preGapIndex) ? static_cast<size_t>(start) : static_cast<size_t>(start) + m_postGapIndex - m_preGapIndex;

    // MOve up first
    if (!right)
    {
        if (absoluteStart < m_preGapIndex && absoluteEnd < m_preGapIndex)
        {
            const std::shared_ptr<LineGapBuffer>& lineLeftOfStart = m_buffer[absoluteStart - 1];

            std::move(m_buffer.begin() + absoluteStart, m_buffer.begin() + absoluteEnd, m_buffer.begin() + absoluteStart - 1);
            m_buffer[absoluteEnd] = std::move(lineLeftOfStart);
        }
        else if (absoluteStart < m_preGapIndex && absoluteEnd >= m_postGapIndex)
        {
            if (absoluteEnd < m_postGapIndex) { endwin(); exit(1); }

            const std::shared_ptr<LineGapBuffer>& lineLeftOfStart = m_buffer[absoluteStart - 1];

            size_t numberOfLinesBeforePreGapIndex = m_preGapIndex - absoluteStart;

            std::move(m_buffer.begin() + absoluteStart, m_buffer.begin() + numberOfLinesBeforePreGapIndex, m_buffer.begin() + absoluteStart - 1);

            m_buffer[m_preGapIndex - 1] = std::move(m_buffer[m_postGapIndex]);

            std::move(m_buffer.begin() + m_postGapIndex + 1, m_buffer.begin() + absoluteEnd, m_buffer.begin() + m_postGapIndex);

            m_buffer[absoluteEnd] = std::move(lineLeftOfStart);
        }
        else
        {
            if (absoluteStart == m_postGapIndex)
            {
                // const std::shared_ptr<LineGapBuffer>& lineLeftOfStart = m_buffer[m_preGapIndex - 1];
                const std::shared_ptr<LineGapBuffer>& lineLeftOfStart = m_buffer[m_postGapIndex];

                m_buffer[m_preGapIndex - 1] = std::move(m_buffer[m_postGapIndex]);

                std::move(m_buffer.begin() + absoluteStart + 1, m_buffer.begin() + absoluteEnd, m_buffer.begin() + absoluteStart);

                m_buffer[absoluteEnd] = std::move(lineLeftOfStart);
            }
            else
            {
                const std::shared_ptr<LineGapBuffer>& lineLeftOfStart = m_buffer[absoluteStart - 1];

                std::move(m_buffer.begin() + absoluteStart, m_buffer.begin() + absoluteEnd, m_buffer.begin() + absoluteStart - 1);
                m_buffer[absoluteEnd] = std::move(lineLeftOfStart);
            }
        }
    }
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
