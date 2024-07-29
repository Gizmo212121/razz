#include "LineGapBuffer.h"

LineGapBuffer::LineGapBuffer(int initialSize)
    : m_buffer(std::vector<char>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
}

LineGapBuffer::LineGapBuffer(int initialSize, const std::string& line)
    : m_buffer(std::vector<char>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
    for (char character : line)
    {
        insertChar(character);
    }
}

void LineGapBuffer::left()
{
    if (m_preGapIndex != 0)
    {
        m_preGapIndex--;
        m_postGapIndex--;
        m_buffer[m_postGapIndex] = m_buffer[m_preGapIndex];
    }
}

void LineGapBuffer::right()
{
    if (m_postGapIndex < m_bufferSize)
    {
        m_buffer[m_preGapIndex] = m_buffer[m_postGapIndex];
        m_preGapIndex++;
        m_postGapIndex++;
    }
}

void LineGapBuffer::insertChar(char character)
{
    if (m_preGapIndex >= m_postGapIndex)
    {
        grow();
    }

    m_buffer[m_preGapIndex] = character;
    m_preGapIndex++;
}

char LineGapBuffer::deleteChar()
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

void LineGapBuffer::grow()
{
    m_buffer.resize(m_bufferSize * 2);
    m_bufferSize *= 2;

    char* bufferBegin = &(*m_buffer.begin());
    char* destination = bufferBegin + m_bufferSize / 2 + m_preGapIndex;
    char* source = bufferBegin + m_preGapIndex;
    size_t n = m_bufferSize / 2 - m_preGapIndex;

    memmove(destination, source, n);

    m_postGapIndex = m_bufferSize / 2 + m_preGapIndex;
}

void LineGapBuffer::printFullLineGapBuffer() const
{
    for (size_t index = 0; index < m_buffer.size(); index++)
    {
        if (index < m_preGapIndex || index >= m_postGapIndex)
        {
            std::cout << m_buffer[index] << ", ";
        }
        else
        {
            std::cout << "_, ";
        }
    }

    std::cout << std::endl;
}

char LineGapBuffer::operator[](size_t index) const
{
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
        std::cout << "File Gap Buffer Info:\n\t" << "Buffer Size: " << m_bufferSize << "\n\t";
        std::cout << "Num characters: " << lineSize() << "\n\t";
        std::cout << "Pre index: " << preGapIndex() << "\n\t";
        std::cout << "Post index: " << postGapIndex() << "\n";
        exit(1);
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

char LineGapBuffer::at(size_t index) const
{
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
        std::cout << "File Gap Buffer Info:\n\t" << "Buffer Size: " << m_bufferSize << "\n\t";
        std::cout << "Num characters: " << lineSize() << "\n\t";
        std::cout << "Pre index: " << preGapIndex() << "\n\t";
        std::cout << "Post index: " << postGapIndex() << "\n";
        // exit(1);
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
