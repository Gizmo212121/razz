#pragma once

#include <stdexcept>
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <memory>

template <class T>
class GapBuffer
{

private:

    std::vector<T> m_buffer;
    size_t m_preGapIndex;
    size_t m_postGapIndex;
    size_t m_bufferSize;

public:

    GapBuffer(int initialSize);
    GapBuffer(int initialSize, const std::string& line);

    static inline int initialBufferSize = 128;

    void left();
    void right();

    void insertItem(T character);
    T deleteItem();

    void grow();

    void printFullGapBuffer() const;

    // Getters
    const std::vector<char>& getLine() const { return m_buffer; }
    size_t preGapIndex() const { return m_preGapIndex; }
    size_t postGapIndex() const { return m_postGapIndex; }
    size_t bufferSize() const { return m_bufferSize; }
    size_t lineSize() const { return m_bufferSize - (m_postGapIndex - m_preGapIndex); }

    T operator [](size_t index) const;

};

template <class T>
GapBuffer<T>::GapBuffer(int initialSize)
    : m_buffer(std::vector<T>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
}

template <class T>
GapBuffer<T>::GapBuffer(int initialSize, const std::string& line)
    : m_buffer(std::vector<char>(initialSize)), m_preGapIndex(0), m_postGapIndex(initialSize), m_bufferSize(initialSize)
{
    for (char character : line)
    {
        insertItem(character);
    }
}

template <class T>
void GapBuffer<T>::left()
{
    if (m_preGapIndex != 0)
    {
        m_preGapIndex--;
        m_postGapIndex--;
        m_buffer[m_postGapIndex] = m_buffer[m_preGapIndex];
    }
}

template <class T>
void GapBuffer<T>::right()
{
    if (m_postGapIndex < m_bufferSize)
    {
        m_buffer[m_preGapIndex] = m_buffer[m_postGapIndex];
        m_preGapIndex++;
        m_postGapIndex++;
    }
}

template <class T>
void GapBuffer<T>::insertItem(T character)
{
    if (m_preGapIndex >= m_postGapIndex)
    {
        grow();
    }

    m_buffer[m_preGapIndex] = character;
    m_preGapIndex++;
}

template <class T>
T GapBuffer<T>::deleteItem()
{
    if (m_preGapIndex > 0)
    {
        return m_buffer[m_preGapIndex--];
    }
    else
    {
        std::cerr << "Deleting at zero!\n";
        exit(1);
    }
}

template <class T>
void GapBuffer<T>::grow()
{
    m_buffer.resize(m_bufferSize * 2);
    m_bufferSize *= 2;

    T* bufferBegin = &(*m_buffer.begin());
    T* destination = bufferBegin + m_bufferSize / 2 + m_preGapIndex;
    T* source = bufferBegin + m_preGapIndex;
    size_t n = m_bufferSize / 2 - m_preGapIndex;

    // std::cout << "Source: " << m_preGapIndex << '\n';
    // std::cout << "Destination: " << m_bufferSize + m_preGapIndex << '\n';
    // std::cout << "N: " << n << '\n';

    memmove(destination, source, n);

    m_postGapIndex = m_bufferSize / 2 + m_preGapIndex;

    // std::cout << "PreGapIndex: " << m_preGapIndex << '\n';
    // std::cout << "PostGapIndex: " << m_postGapIndex << '\n';
}

template <class T>
void GapBuffer<T>::printFullGapBuffer() const
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

template <class T>
T GapBuffer<T>::operator[](size_t index) const
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
