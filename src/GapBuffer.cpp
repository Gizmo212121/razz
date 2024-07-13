#include "GapBuffer.h"

#include <cstring>
#include <iostream>

GapBuffer::GapBuffer(int initialSize)
    : m_buffer(std::vector<char>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
}

void GapBuffer::left()
{
    if (m_preGapIndex != 0)
    {
        m_preGapIndex--;
        m_postGapIndex--;
        m_buffer[m_postGapIndex] = m_buffer[m_preGapIndex];
    }
}

void GapBuffer::right()
{
    if (m_postGapIndex < m_bufferSize)
    {
        m_buffer[m_preGapIndex] = m_buffer[m_postGapIndex];
        m_preGapIndex++;
        m_postGapIndex++;
    }
}

void GapBuffer::insertChar(char character)
{
    if (m_preGapIndex >= m_postGapIndex)
    {
        grow();
    }

    m_buffer[m_preGapIndex] = character;
    m_preGapIndex++;
}

char GapBuffer::deleteChar()
{
    if (m_preGapIndex > 0)
    {
        return m_buffer[m_preGapIndex--];
    }
    else
    {
        return '\0';
    }
}

void GapBuffer::grow()
{
    m_buffer.resize(m_bufferSize * 2);
    m_bufferSize *= 2;

    char* bufferBegin = &(*m_buffer.begin());
    char* destination = bufferBegin + m_bufferSize / 2 + m_preGapIndex;
    char* source = bufferBegin + m_preGapIndex;
    size_t n = m_bufferSize / 2 - m_preGapIndex;

    // std::cout << "Source: " << m_preGapIndex << '\n';
    // std::cout << "Destination: " << m_bufferSize + m_preGapIndex << '\n';
    // std::cout << "N: " << n << '\n';

    memmove(destination, source, n);

    m_postGapIndex = m_bufferSize / 2 + m_preGapIndex;

    // std::cout << "PreGapIndex: " << m_preGapIndex << '\n';
    // std::cout << "PostGapIndex: " << m_postGapIndex << '\n';
}

void GapBuffer::printFullGapBuffer() const
{
    // std::cout << "Buffer size: " << m_buffer.size() << '\n';
    for (size_t index = 0; index < m_buffer.size(); index++)
    {
        if (index < m_preGapIndex || index >= m_postGapIndex)
        {
            std::cout << m_buffer[index] << ", ";
        }
        else
        {
            // std::cout << m_buffer[index] << ", ";
            std::cout << "_, ";
        }
    }
    // std::cout << "Last element: " << m_buffer[m_bufferSize] << '\n';
    // std::cout << "After last element: " << m_buffer[m_buffer.size()];

    std::cout << std::endl;
}
