#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(size_t maxSize)
    : m_maxSize(maxSize), m_buffer()
{
}

void CircularBuffer::add(int x)
{
    if (m_buffer.size() == m_maxSize)
    {
        m_buffer.pop_back();
    }

    m_buffer.push_front(x);
}

int CircularBuffer::operator [] (size_t index) const
{
    if (index >= m_buffer.size())
    {
        return -1;
    }
    else
    {
        return m_buffer[index];
    }
}
