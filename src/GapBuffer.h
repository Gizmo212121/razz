#pragma once

#include <vector>
#include <stdio.h>
#include <string>

class GapBuffer
{

private:

    std::vector<char> m_buffer;
    size_t m_preGapIndex;
    size_t m_postGapIndex;
    size_t m_bufferSize;

public:

    GapBuffer(int initialSize);
    GapBuffer(int initialSize, const std::string& line);

    static inline int initialBufferSize = 128;

    void left();
    void right();
    void insertChar(char character);
    char deleteChar();
    void grow();

    void printFullGapBuffer() const;

    // Getters
    const std::vector<char>& getLine() const { return m_buffer; }
    size_t preGapIndex() const { return m_preGapIndex; }
    size_t postGapIndex() const { return m_postGapIndex; }
    size_t bufferSize() const { return m_bufferSize; }
    size_t lineSize() const { return m_bufferSize - (m_postGapIndex - m_preGapIndex); }

    char operator [](size_t index) const;

};
