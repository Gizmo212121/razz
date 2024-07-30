#pragma once

#include "Includes.h"

class CircularBuffer
{

private:

    size_t m_maxSize;
    std::deque<int> m_buffer;

public:

    CircularBuffer(size_t maxSize);

    int operator [] (size_t index) const;

    void add(int x);

    size_t maxSize() const { return m_maxSize; }

};
