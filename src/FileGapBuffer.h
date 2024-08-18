#include "LineGapBuffer.h"

class FileGapBuffer
{

private:

    std::vector<std::shared_ptr<LineGapBuffer>> m_buffer;
    size_t m_preGapIndex;
    size_t m_postGapIndex;
    size_t m_bufferSize;

public:

    FileGapBuffer(int initialSize);

    void up();
    void down();

    void insertLine(const std::shared_ptr<LineGapBuffer>& line);
    std::shared_ptr<LineGapBuffer> deleteLine();

    void grow();

    void swapLinesInRange(bool down, int start, int end);

    // Getters
    const std::vector<std::shared_ptr<LineGapBuffer>>& getVectorOfSharedPtrsToLineGapBuffers() const { return m_buffer; }
    size_t preGapIndex() const { return m_preGapIndex; }
    size_t postGapIndex() const { return m_postGapIndex; }
    size_t bufferSize() const { return m_bufferSize; }
    size_t numberOfLines() const { return m_bufferSize - (m_postGapIndex - m_preGapIndex); }

    const std::shared_ptr<LineGapBuffer>& operator [](size_t index) const;

};
