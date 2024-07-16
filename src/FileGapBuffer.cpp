#include "FileGapBuffer.h"

#include <utility>

FileGapBuffer::FileGapBuffer()
    : m_buffer(std::vector<std::shared_ptr<LineGapBuffer>>()), m_preGapIndex(0), m_postGapIndex(0), m_bufferSize(0)
{
}

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
        m_buffer[m_postGapIndex] = m_buffer[m_preGapIndex];
    }
}

void FileGapBuffer::down()
{
    if (m_postGapIndex < m_bufferSize)
    {
        m_buffer[m_preGapIndex] = m_buffer[m_postGapIndex];
        m_preGapIndex++;
        m_postGapIndex++;
    }
}

void FileGapBuffer::insertLine(const std::shared_ptr<LineGapBuffer>& line)
{
    if (m_preGapIndex >= m_postGapIndex)
    {
        grow();
    }

    m_buffer[m_preGapIndex] = line;
    m_preGapIndex++;
}

std::shared_ptr<LineGapBuffer> FileGapBuffer::deleteLine()
{
    if (m_preGapIndex > 0)
    {
        return m_buffer[m_preGapIndex--];
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

    // std::shared_ptr<LineGapBuffer>* bufferBegin = &(*m_buffer.begin());
    // std::shared_ptr<LineGapBuffer>* destination = bufferBegin + m_bufferSize / 2 + m_preGapIndex;
    // std::shared_ptr<LineGapBuffer>* source = bufferBegin + m_preGapIndex;
    // size_t n = m_bufferSize / 2 - m_preGapIndex;

    // std::move(m_buffer.begin() + m_bufferSize / 2 + m_preGapIndex, )
    std::move(m_buffer.begin(), m_buffer.begin() + m_preGapIndex, newBuffer.begin());
    std::move(m_buffer.begin() + m_postGapIndex, m_buffer.end(), newBuffer.begin() + newSize / 2);
    // std::memmove(destination, source, n);

    m_postGapIndex = newSize / 2 + m_preGapIndex;

    m_buffer = std::move(newBuffer);
    m_bufferSize = newSize;
}

const std::shared_ptr<LineGapBuffer>& FileGapBuffer::operator [](size_t index) const
{
    assert(index < m_bufferSize - m_postGapIndex + m_preGapIndex);

    if (index < m_preGapIndex)
    {
        return m_buffer[index];
    }
    else
    {
        return m_buffer[index + m_postGapIndex - m_preGapIndex];
    }
}
