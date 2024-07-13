#pragma once

#include <vector>
#include <stdio.h>

class GapBuffer
{

private:

    std::vector<char> m_buffer;
    size_t m_preGapIndex;
    size_t m_postGapIndex;
    size_t m_bufferSize;

public:

    GapBuffer(int initialSize);

    void left();
    void right();
    void insertChar(char character);
    char deleteChar();
    void grow();

    void printFullGapBuffer() const;

    // Getters
    size_t preGapIndex() const { return m_preGapIndex; }
    size_t postGapIndex() const { return m_postGapIndex; }
    size_t bufferSize() const { return m_bufferSize; }

};
